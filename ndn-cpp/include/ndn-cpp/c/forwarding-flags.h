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

/* Note: This file should be named forward-flags-types.h (like data-types.h) but
 * we leave it as forwarding-flags.h for backwards compatibility with the
 * include directory of old installations.
 */

#ifndef NDN_FORWARDING_FLAGS_H
#define NDN_FORWARDING_FLAGS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An ndn_ForwardingFlags object holds the flags which specify how the forwarding daemon should forward an interest for
 * a registered prefix.  We use a separate ForwardingFlags object to retain future compatibility if the daemon forwarding
 * bits are changed, amended or deprecated.
 */
struct ndn_ForwardingFlags {
  int active; /**< 1 if the flag is set, 0 if cleared. */
  int childInherit;
  int advertise;
  int last;
  int capture;
  int local;
  int tap;
  int captureOk;
};

#ifdef __cplusplus
}
#endif

#endif
