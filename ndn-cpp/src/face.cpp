/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "node.hpp"
#include <ndn-cpp/transport/tcp-transport.hpp>
#include <ndn-cpp/transport/unix-transport.hpp>
#include <ndn-cpp/face.hpp>

using namespace std;
//
//using namespace boost;
using namespace ndn::func_lib;

namespace ndn {

/**
 * If the forwarder's Unix socket file path exists, then return the file path.
 * Otherwise return an empty string.
 * @return The Unix socket file path to use, or an empty string.
 */
static string getUnixSocketFilePathForLocalhost()
{
  string filePath = "/var/run/nfd.sock";
  if (::access(filePath.c_str(), R_OK) == 0)
    return filePath;
  else {
    filePath = "/tmp/.ndnd.sock";
    if (::access(filePath.c_str(), R_OK) == 0)
      return filePath;
    else
      return "";
  }
}

static ptr_lib::shared_ptr<Transport> getDefaultTransport()
{
  if (getUnixSocketFilePathForLocalhost() == "")
    return ptr_lib::make_shared<TcpTransport>();
  else
    return ptr_lib::make_shared<UnixTransport>();
}

static ptr_lib::shared_ptr<Transport::ConnectionInfo> getDefaultConnectionInfo()
{
  string filePath = getUnixSocketFilePathForLocalhost();
  if (filePath == "")
    return ptr_lib::make_shared<TcpTransport::ConnectionInfo>("localhost");
  else
    return ptr_lib::shared_ptr<UnixTransport::ConnectionInfo>
      (new UnixTransport::ConnectionInfo(filePath.c_str()));
}

Face::Face(const ptr_lib::shared_ptr<Transport>& transport, const ptr_lib::shared_ptr<const Transport::ConnectionInfo>& connectionInfo)
: node_(new Node(transport, connectionInfo)), commandKeyChain_(0)
{
}

Face::Face(const char *host, unsigned short port)
: node_(new Node(ptr_lib::shared_ptr<TcpTransport>(new TcpTransport()),
                 ptr_lib::make_shared<TcpTransport::ConnectionInfo>(host, port))),
  commandKeyChain_(0)
{
}

Face::Face()
: node_(new Node(getDefaultTransport(), getDefaultConnectionInfo())),
  commandKeyChain_(0)
{
}

Face::~Face()
{
  delete node_;
}

boost::uint64_t
Face::expressInterest
  (const Interest& interest, const OnData& onData, const OnTimeout& onTimeout, WireFormat& wireFormat)
{
  return node_->expressInterest(interest, onData, onTimeout, wireFormat);
}

boost::uint64_t
Face::expressInterest
  (const Name& name, const Interest *interestTemplate, const OnData& onData, const OnTimeout& onTimeout,
   WireFormat& wireFormat)
{
  if (interestTemplate) {
    // Copy the interestTemplate.
    Interest interest(*interestTemplate);
    interest.setName(name);
    return node_->expressInterest(interest, onData, onTimeout, wireFormat);
  }
  else
    return node_->expressInterest(Interest(name, 4000.0), onData, onTimeout, wireFormat);
}

void
Face::removePendingInterest(boost::uint64_t pendingInterestId)
{
  node_->removePendingInterest(pendingInterestId);
}

void
Face::makeCommandInterest(Interest& interest, WireFormat& wireFormat)
{
  node_->makeCommandInterest
    (interest, *commandKeyChain_, commandCertificateName_, wireFormat);
}

boost::uint64_t
Face::registerPrefix
  (const Name& prefix, const OnInterestCallback& onInterest,
   const OnRegisterFailed& onRegisterFailed, const ForwardingFlags& flags,
   WireFormat& wireFormat)
{
  return node_->registerPrefix
    (prefix, onInterest, onRegisterFailed, flags, wireFormat, *commandKeyChain_,
     commandCertificateName_, this);
}

boost::uint64_t
Face::registerPrefix
  (const Name& prefix, const OnInterest& onInterest,
   const OnRegisterFailed& onRegisterFailed,
   const ForwardingFlags& flags, WireFormat& wireFormat)
{
  // Wrap the deprecated OnInterest in an OnInterestCallback.
  // TODO: After we remove the registerPrefix with the deprecated OnInterest,
  // we can remove the explicit cast to OnInterestCallback (needed for boost).
  return registerPrefix
    (prefix,
     (const OnInterestCallback&)bind(&Face::onInterestWrapper, _1, _2, _3, _4, _5, onInterest),
     onRegisterFailed, flags, wireFormat);
}

void
Face::onInterestWrapper
  (const ptr_lib::shared_ptr<const Name>& prefix,
   const ptr_lib::shared_ptr<const Interest>& interest, Face& face,
   boost::uint64_t interestFilterId,
   const ptr_lib::shared_ptr<const InterestFilter>& filter,
   const OnInterest callerOnInterest)
{
  callerOnInterest
    (prefix, interest, *face.node_->getTransport(), interestFilterId);
}

void
Face::removeRegisteredPrefix(boost::uint64_t registeredPrefixId)
{
  node_->removeRegisteredPrefix(registeredPrefixId);
}

boost::uint64_t
Face::setInterestFilter
  (const InterestFilter& filter, const OnInterestCallback& onInterest)
{
  return node_->setInterestFilter(filter, onInterest, this);
}

boost::uint64_t
Face::setInterestFilter(const Name& prefix, const OnInterestCallback& onInterest)
{
  return node_->setInterestFilter(InterestFilter(prefix), onInterest, this);
}

void
Face::unsetInterestFilter(boost::uint64_t interestFilterId)
{
  node_->unsetInterestFilter(interestFilterId);
}

void
Face::putData(const Data& data, WireFormat& wireFormat)
{
  node_->putData(data, wireFormat);
}

void
Face::send(const uint8_t *encoding, size_t encodingLength)
{
  node_->send(encoding, encodingLength);
}

void
Face::processEvents()
{
  // Just call Node's processEvents.
  node_->processEvents();
}

bool
Face::isLocal()
{
  return node_->isLocal();
}

void
Face::shutdown()
{
  node_->shutdown();
}

}
