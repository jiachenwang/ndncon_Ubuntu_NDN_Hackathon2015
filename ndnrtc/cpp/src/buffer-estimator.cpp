//
//  buffer-estimator.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#include "buffer-estimator.h"
#include "rtt-estimation.h"

using namespace boost;
using namespace ndnrtc::new_api;

// minimal buffer size in milliseconds
const boost::int64_t BufferEstimator::MinBufferSizeMs = 250;

//******************************************************************************
#pragma mark - construction/destruction
BufferEstimator::BufferEstimator(const shared_ptr<RttEstimation>& rttEstimation,
                                 boost::int64_t minBufferSizeMs):
rttEstimation_(rttEstimation),
minBufferSizeMs_(minBufferSizeMs)
{
}

//******************************************************************************
#pragma mark - public
void
BufferEstimator::setProducerRate(double producerRate)
{
}

boost::int64_t
BufferEstimator::getTargetSize()
{
    double rttEstimate = rttEstimation_->getCurrentEstimation();
    
    // we set buffer target size to be 2*RTT or
    // minimal buffer size specified by user (or default)
    
    if (rttEstimate*2 > minBufferSizeMs_)
        return rttEstimate*2;
    
    return minBufferSizeMs_;
}