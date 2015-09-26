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

#ifndef NDN_FORWARDING_ENTRY_H
#define NDN_FORWARDING_ENTRY_H

#include <math.h>
#include <ndn-cpp/c/common.h>
#include "forwarding-flags-impl.h"
#include "name.h"
#include "publisher-public-key-digest.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ndn_ForwardingEntryFlags_ACTIVE         = 1,
  ndn_ForwardingEntryFlags_CHILD_INHERIT  = 2,
  ndn_ForwardingEntryFlags_ADVERTISE      = 4,
  ndn_ForwardingEntryFlags_LAST           = 8,
  ndn_ForwardingEntryFlags_CAPTURE       = 16,
  ndn_ForwardingEntryFlags_LOCAL         = 32,
  ndn_ForwardingEntryFlags_TAP           = 64,
  ndn_ForwardingEntryFlags_CAPTURE_OK   = 128
} ndn_ForwardingEntryFlags;

/**
 * An ndn_ForwardingEntry holds fields for a ForwardingEntry which is used to register a prefix with a hub.
 */
struct ndn_ForwardingEntry {
  struct ndn_Blob action;   /**< A Blob whose value is a pointer to a pre-allocated buffer.  0 for none. */
  struct ndn_Name prefix;
  struct ndn_PublisherPublicKeyDigest publisherPublicKeyDigest;
  int faceId;               /**< -1 for none. */
  struct ndn_ForwardingFlags forwardingFlags;
  ndn_Milliseconds freshnessPeriod; /**< -1 for none. */
};

/**
 * Initialize an ndn_ForwardingEntry struct with the pre-allocated prefixNameComponents,
 * and defaults for all the values.
 * @param self pointer to the ndn_ForwardingEntry struct
 * @param prefixNameComponents the pre-allocated array of ndn_NameComponent
 * @param maxPrefixNameComponents the number of elements in the allocated prefixNameComponents array
 */
static __inline void ndn_ForwardingEntry_initialize
  (struct ndn_ForwardingEntry *self, struct ndn_NameComponent *prefixNameComponents, size_t maxPrefixNameComponents)
{
  ndn_Blob_initialize(&self->action, 0, 0);
  ndn_Name_initialize(&self->prefix, prefixNameComponents, maxPrefixNameComponents);
  ndn_PublisherPublicKeyDigest_initialize(&self->publisherPublicKeyDigest);
  self->faceId = -1;
  ndn_ForwardingFlags_initialize(&self->forwardingFlags);
  self->freshnessPeriod = -1.0;
}

/**
 * @deprecated Use freshnessPeriod.
 */
static __inline int ndn_ForwardingEntry_getFreshnessSeconds
  (const struct ndn_ForwardingEntry *self)
{
  return self->freshnessPeriod < 0 ? -1 : (int)round(self->freshnessPeriod / 1000.0);
}

/**
 * @deprecated Use freshnessPeriod.
 */
static __inline void ndn_ForwardingEntry_setFreshnessSeconds
  (struct ndn_ForwardingEntry *self, int freshnessSeconds)
{
  self->freshnessPeriod = freshnessSeconds < 0 ? -1.0 : (double)freshnessSeconds * 1000.0;
}

#ifdef __cplusplus
}
#endif

#endif
