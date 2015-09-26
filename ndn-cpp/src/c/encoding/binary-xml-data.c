/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
 * Derived from ContentObject.js by Meki Cheraoui.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with the additional exemption that
 * compiling, linking, and/or using OpenSSL is allowed.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * A copy of the GNU Lesser General Public License is in the file COPYING.
 */

#include "binary-xml-name.h"
#include "binary-xml-publisher-public-key-digest.h"
#include "binary-xml-data.h"
#include "binary-xml-key-locator.h"

static ndn_Error encodeSignature(struct ndn_Signature *signature, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;

  if (signature->type != ndn_SignatureType_Sha256WithRsaSignature)
    return NDN_ERROR_encodeSignatureInfo_unrecognized_SignatureType;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_Signature)))
    return error;

  // TODO: Check if digestAlgorithm is the same as the default, and skip it, otherwise encode it as UDATA.

  if ((error = ndn_BinaryXmlEncoder_writeOptionalBlobDTagElement(encoder, ndn_BinaryXml_DTag_Witness, &signature->witness)))
    return error;

  // Require a signature.
  if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_SignatureBits, &signature->signature)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

static ndn_Error decodeSignature(struct ndn_Signature *signature, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_Signature)))
    return error;

  /* TODO: digestAlgorithm as UDATA */ signature->digestAlgorithm.value = 0; signature->digestAlgorithm.length = 0;

  if ((error = ndn_BinaryXmlDecoder_readOptionalBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_Witness, 0, &signature->witness)))
    return error;

  // Require a signature.
  if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_SignatureBits, 0, &signature->signature)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  signature->type = ndn_SignatureType_Sha256WithRsaSignature;

  return NDN_ERROR_success;
}

static ndn_Error encodeSignedInfo(struct ndn_Signature *signature, struct ndn_MetaInfo *metaInfo, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;

  if (signature->type != ndn_SignatureType_Sha256WithRsaSignature)
    return NDN_ERROR_encodeSignatureInfo_unrecognized_SignatureType;

  if ((int)metaInfo->type < 0)
    return NDN_ERROR_success;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_SignedInfo)))
    return error;

  if (signature->publisherPublicKeyDigest.publisherPublicKeyDigest.length > 0 &&
      signature->publisherPublicKeyDigest.publisherPublicKeyDigest.value) {
    // We have a publisherPublicKeyDigest, so use it.
    if ((error = ndn_encodeBinaryXmlPublisherPublicKeyDigest(&signature->publisherPublicKeyDigest, encoder)))
      return error;
  }
  else {
    if (signature->keyLocator.type == ndn_KeyLocatorType_KEY_LOCATOR_DIGEST &&
        signature->keyLocator.keyData.length > 0) {
      // We have a TLV-style KEY_LOCATOR_DIGEST, so encode as the
      //   publisherPublicKeyDigest.
      if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement
          (encoder, ndn_BinaryXml_DTag_PublisherPublicKeyDigest,
           &signature->keyLocator.keyData)))
        return error;
    }
  }

  if ((error = ndn_BinaryXmlEncoder_writeOptionalTimeMillisecondsDTagElement
      (encoder, ndn_BinaryXml_DTag_Timestamp, metaInfo->timestampMilliseconds)))
    return error;

  if (!((int)metaInfo->type < 0 || metaInfo->type == ndn_ContentType_DATA)) {
    // Not the default of DATA, so we need to encode the type.
    struct ndn_Blob typeBytes;
    typeBytes.length = 3;
    if (metaInfo->type == ndn_ContentType_ENCR)
      typeBytes.value = (uint8_t *)"\x10\xD0\x91";
    else if (metaInfo->type == ndn_ContentType_GONE)
      typeBytes.value = (uint8_t *)"\x18\xE3\x44";
    else if (metaInfo->type == ndn_ContentType_KEY)
      typeBytes.value = (uint8_t *)"\x28\x46\x3F";
    else if (metaInfo->type == ndn_ContentType_LINK)
      typeBytes.value = (uint8_t *)"\x2C\x83\x4A";
    else if (metaInfo->type == ndn_ContentType_NACK)
      typeBytes.value = (uint8_t *)"\x34\x00\x8A";
    else
      return NDN_ERROR_unrecognized_ndn_ContentType;

    if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_Type, &typeBytes)))
      return error;
  }

  if ((error = ndn_BinaryXmlEncoder_writeOptionalUnsignedDecimalIntDTagElement
      (encoder, ndn_BinaryXml_DTag_FreshnessSeconds, ndn_MetaInfo_getFreshnessSeconds(metaInfo))))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeOptionalBlobDTagElement
      (encoder, ndn_BinaryXml_DTag_FinalBlockID, &metaInfo->finalBlockId.value)))
    return error;

  // This will skip encoding if there is no key locator.
  if ((error = ndn_encodeBinaryXmlKeyLocator(&signature->keyLocator, encoder)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

static ndn_Error decodeSignedInfo(struct ndn_Signature *signature, struct ndn_MetaInfo *metaInfo, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  struct ndn_Blob typeBytes;
  int freshnessSeconds;

  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_SignedInfo)))
    return error;

  if ((error = ndn_decodeOptionalBinaryXmlPublisherPublicKeyDigest(&signature->publisherPublicKeyDigest, decoder)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalTimeMillisecondsDTagElement
       (decoder, ndn_BinaryXml_DTag_Timestamp, &metaInfo->timestampMilliseconds)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readOptionalBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_Type, 0, &typeBytes)))
    return error;
  if (typeBytes.length == 0)
    // The default Type is DATA.
    metaInfo->type = ndn_ContentType_DATA;
  else if (typeBytes.length == 3) {
    // All the recognized content types are 3 bytes.
    if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x0C\x04\xC0", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_DATA;
    else if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x10\xD0\x91", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_ENCR;
    else if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x18\xE3\x44", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_GONE;
    else if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x28\x46\x3F", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_KEY;
    else if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x2C\x83\x4A", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_LINK;
    else if (ndn_memcmp(typeBytes.value, (uint8_t *)"\x34\x00\x8A", typeBytes.length) == 0)
      metaInfo->type = ndn_ContentType_NACK;
    else
      return NDN_ERROR_unrecognized_ndn_ContentType;
  }
  else
    return NDN_ERROR_unrecognized_ndn_ContentType;

  if ((error = ndn_BinaryXmlDecoder_readOptionalUnsignedIntegerDTagElement
      (decoder, ndn_BinaryXml_DTag_FreshnessSeconds, &freshnessSeconds)))
    return error;
  ndn_MetaInfo_setFreshnessSeconds(metaInfo, freshnessSeconds);

  if ((error = ndn_BinaryXmlDecoder_readOptionalBinaryDTagElement
      (decoder, ndn_BinaryXml_DTag_FinalBlockID, 0, &metaInfo->finalBlockId.value)))
    return error;

  if ((error = ndn_decodeOptionalBinaryXmlKeyLocator(&signature->keyLocator, decoder)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_encodeBinaryXmlData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_BinaryXmlEncoder *encoder)
{
  ndn_Error error;
  size_t dummyBeginOffset, dummyEndOffset;

  if ((error = ndn_BinaryXmlEncoder_writeElementStartDTag(encoder, ndn_BinaryXml_DTag_ContentObject)))
    return error;

  if ((error = encodeSignature(&data->signature, encoder)))
    return error;

  *signedPortionBeginOffset = encoder->offset;

  if ((error = ndn_encodeBinaryXmlName
       (&data->name, &dummyBeginOffset, &dummyEndOffset, encoder)))
    return error;

  if ((error = encodeSignedInfo(&data->signature, &data->metaInfo, encoder)))
    return error;

  if ((error = ndn_BinaryXmlEncoder_writeBlobDTagElement(encoder, ndn_BinaryXml_DTag_Content, &data->content)))
    return error;

  *signedPortionEndOffset = encoder->offset;

  if ((error = ndn_BinaryXmlEncoder_writeElementClose(encoder)))
    return error;

  return NDN_ERROR_success;
}

ndn_Error ndn_decodeBinaryXmlData
  (struct ndn_Data *data, size_t *signedPortionBeginOffset, size_t *signedPortionEndOffset, struct ndn_BinaryXmlDecoder *decoder)
{
  ndn_Error error;
  int gotExpectedTag;
  size_t dummyBeginOffset, dummyEndOffset;

  if ((error = ndn_BinaryXmlDecoder_readElementStartDTag(decoder, ndn_BinaryXml_DTag_ContentObject)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_Signature, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    if ((error = decodeSignature(&data->signature, decoder)))
      return error;
  }
  else
    ndn_Signature_initialize(&data->signature, data->signature.keyLocator.keyName.components, data->signature.keyLocator.keyName.maxComponents);

  *signedPortionBeginOffset = decoder->offset;

  if ((error = ndn_decodeBinaryXmlName
       (&data->name, &dummyBeginOffset, &dummyEndOffset, decoder)))
    return error;

  if ((error = ndn_BinaryXmlDecoder_peekDTag(decoder, ndn_BinaryXml_DTag_SignedInfo, &gotExpectedTag)))
    return error;
  if (gotExpectedTag) {
    if ((error = decodeSignedInfo(&data->signature, &data->metaInfo, decoder)))
      return error;
  }
  else
    ndn_MetaInfo_initialize(&data->metaInfo);

  // Require a Content element, but set allowNull to allow a missing BLOB.
  if ((error = ndn_BinaryXmlDecoder_readBinaryDTagElement(decoder, ndn_BinaryXml_DTag_Content, 1, &data->content)))
    return error;

  *signedPortionEndOffset = decoder->offset;

  if ((error = ndn_BinaryXmlDecoder_readElementClose(decoder)))
    return error;

  return NDN_ERROR_success;
}
