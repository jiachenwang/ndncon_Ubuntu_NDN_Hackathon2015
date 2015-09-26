/**
 * Copyright (C) 2013-2015 Regents of the University of California.
 * @author: Jeff Thompson <jefft0@remap.ucla.edu>
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

#ifndef NDN_BINARYXML_H
#define NDN_BINARYXML_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
  ndn_BinaryXml_EXT = 0x00,
  ndn_BinaryXml_TAG = 0x01,
  ndn_BinaryXml_DTAG = 0x02,
  ndn_BinaryXml_ATTR = 0x03,
  ndn_BinaryXml_DATTR = 0x04,
  ndn_BinaryXml_BLOB = 0x05,
  ndn_BinaryXml_UDATA = 0x06,
  ndn_BinaryXml_CLOSE = 0x0,

  ndn_BinaryXml_TT_BITS = 3,
  ndn_BinaryXml_TT_MASK = ((1 << ndn_BinaryXml_TT_BITS) - 1),
  ndn_BinaryXml_TT_VALUE_BITS = 4,
  ndn_BinaryXml_TT_VALUE_MASK = ((1 << (ndn_BinaryXml_TT_VALUE_BITS)) - 1),
  ndn_BinaryXml_REGULAR_VALUE_BITS = 7,
  ndn_BinaryXml_REGULAR_VALUE_MASK = ((1 << ndn_BinaryXml_REGULAR_VALUE_BITS) - 1),
  ndn_BinaryXml_TT_FINAL = 0x80,

  ndn_BinaryXml_DTag_Any = 13,
  ndn_BinaryXml_DTag_Name = 14,
  ndn_BinaryXml_DTag_Component = 15,
  ndn_BinaryXml_DTag_Certificate = 16,
  ndn_BinaryXml_DTag_Collection = 17,
  ndn_BinaryXml_DTag_CompleteName = 18,
  ndn_BinaryXml_DTag_Content = 19,
  ndn_BinaryXml_DTag_SignedInfo = 20,
  ndn_BinaryXml_DTag_ContentDigest = 21,
  ndn_BinaryXml_DTag_ContentHash = 22,
  ndn_BinaryXml_DTag_Count = 24,
  ndn_BinaryXml_DTag_Header = 25,
  ndn_BinaryXml_DTag_Interest = 26,  /* 20090915 */
  ndn_BinaryXml_DTag_Key = 27,
  ndn_BinaryXml_DTag_KeyLocator = 28,
  ndn_BinaryXml_DTag_KeyName = 29,
  ndn_BinaryXml_DTag_Length = 30,
  ndn_BinaryXml_DTag_Link = 31,
  ndn_BinaryXml_DTag_LinkAuthenticator = 32,
  ndn_BinaryXml_DTag_NameComponentCount = 33,  /* DeprecatedInInterest */
  ndn_BinaryXml_DTag_RootDigest = 36,
  ndn_BinaryXml_DTag_Signature = 37,
  ndn_BinaryXml_DTag_Start = 38,
  ndn_BinaryXml_DTag_Timestamp = 39,
  ndn_BinaryXml_DTag_Type = 40,
  ndn_BinaryXml_DTag_Nonce = 41,
  ndn_BinaryXml_DTag_Scope = 42,
  ndn_BinaryXml_DTag_Exclude = 43,
  ndn_BinaryXml_DTag_Bloom = 44,
  ndn_BinaryXml_DTag_BloomSeed = 45,
  ndn_BinaryXml_DTag_AnswerOriginKind = 47,
  ndn_BinaryXml_DTag_InterestLifetime = 48,
  ndn_BinaryXml_DTag_Witness = 53,
  ndn_BinaryXml_DTag_SignatureBits = 54,
  ndn_BinaryXml_DTag_DigestAlgorithm = 55,
  ndn_BinaryXml_DTag_BlockSize = 56,
  ndn_BinaryXml_DTag_FreshnessSeconds = 58,
  ndn_BinaryXml_DTag_FinalBlockID = 59,
  ndn_BinaryXml_DTag_PublisherPublicKeyDigest = 60,
  ndn_BinaryXml_DTag_PublisherCertificateDigest = 61,
  ndn_BinaryXml_DTag_PublisherIssuerKeyDigest = 62,
  ndn_BinaryXml_DTag_PublisherIssuerCertificateDigest = 63,
  ndn_BinaryXml_DTag_ContentObject = 64,  /* 20090915 */
  ndn_BinaryXml_DTag_WrappedKey = 65,
  ndn_BinaryXml_DTag_WrappingKeyIdentifier = 66,
  ndn_BinaryXml_DTag_WrapAlgorithm = 67,
  ndn_BinaryXml_DTag_KeyAlgorithm = 68,
  ndn_BinaryXml_DTag_Label = 69,
  ndn_BinaryXml_DTag_EncryptedKey = 70,
  ndn_BinaryXml_DTag_EncryptedNonceKey = 71,
  ndn_BinaryXml_DTag_WrappingKeyName = 72,
  ndn_BinaryXml_DTag_Action = 73,
  ndn_BinaryXml_DTag_FaceID = 74,
  ndn_BinaryXml_DTag_IPProto = 75,
  ndn_BinaryXml_DTag_Host = 76,
  ndn_BinaryXml_DTag_Port = 77,
  ndn_BinaryXml_DTag_MulticastInterface = 78,
  ndn_BinaryXml_DTag_ForwardingFlags = 79,
  ndn_BinaryXml_DTag_FaceInstance = 80,
  ndn_BinaryXml_DTag_ForwardingEntry = 81,
  ndn_BinaryXml_DTag_MulticastTTL = 82,
  ndn_BinaryXml_DTag_MinSuffixComponents = 83,
  ndn_BinaryXml_DTag_MaxSuffixComponents = 84,
  ndn_BinaryXml_DTag_ChildSelector = 85,
  ndn_BinaryXml_DTag_RepositoryInfo = 86,
  ndn_BinaryXml_DTag_Version = 87,
  ndn_BinaryXml_DTag_RepositoryVersion = 88,
  ndn_BinaryXml_DTag_GlobalPrefix = 89,
  ndn_BinaryXml_DTag_LocalName = 90,
  ndn_BinaryXml_DTag_Policy = 91,
  ndn_BinaryXml_DTag_Namespace = 92,
  ndn_BinaryXml_DTag_GlobalPrefixName = 93,
  ndn_BinaryXml_DTag_PolicyVersion = 94,
  ndn_BinaryXml_DTag_KeyValueSet = 95,
  ndn_BinaryXml_DTag_KeyValuePair = 96,
  ndn_BinaryXml_DTag_IntegerValue = 97,
  ndn_BinaryXml_DTag_DecimalValue = 98,
  ndn_BinaryXml_DTag_StringValue = 99,
  ndn_BinaryXml_DTag_BinaryValue = 100,
  ndn_BinaryXml_DTag_NameValue = 101,
  ndn_BinaryXml_DTag_Entry = 102,
  ndn_BinaryXml_DTag_ACL = 103,
  ndn_BinaryXml_DTag_ParameterizedName = 104,
  ndn_BinaryXml_DTag_Prefix = 105,
  ndn_BinaryXml_DTag_Suffix = 106,
  ndn_BinaryXml_DTag_Root = 107,
  ndn_BinaryXml_DTag_ProfileName = 108,
  ndn_BinaryXml_DTag_Parameters = 109,
  ndn_BinaryXml_DTag_InfoString = 110,
  ndn_BinaryXml_DTag_StatusResponse = 112,
  ndn_BinaryXml_DTag_StatusCode = 113,
  ndn_BinaryXml_DTag_StatusText = 114,
  ndn_BinaryXml_DTag_SyncNode = 115,
  ndn_BinaryXml_DTag_SyncNodeKind = 116,
  ndn_BinaryXml_DTag_SyncNodeElement = 117,
  ndn_BinaryXml_DTag_SyncVersion = 118,
  ndn_BinaryXml_DTag_SyncNodeElements = 119,
  ndn_BinaryXml_DTag_SyncContentHash = 120,
  ndn_BinaryXml_DTag_SyncLeafCount = 121,
  ndn_BinaryXml_DTag_SyncTreeDepth = 122,
  ndn_BinaryXml_DTag_SyncByteCount = 123,
  ndn_BinaryXml_DTag_SyncConfigSlice = 124,
  ndn_BinaryXml_DTag_SyncConfigSliceList = 125,
  ndn_BinaryXml_DTag_SyncConfigSliceOp = 126,
  ndn_BinaryXml_DTag_SyncNodeDeltas = 127,
  ndn_BinaryXml_DTag_SequenceNumber = 256,
  ndn_BinaryXml_DTag_NDNProtocolDataUnit = 20587744 // the encoded empty element, viewed as a string is "NDN\202\000"
};

#ifdef __cplusplus
}
#endif

#endif
