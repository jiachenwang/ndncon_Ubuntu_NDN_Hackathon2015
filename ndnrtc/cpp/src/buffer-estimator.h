//
//  buffer-estimator.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#ifndef __ndnrtc__buffer_estimator__
#define __ndnrtc__buffer_estimator__

#include "ndnrtc-common.h"

namespace ndnrtc {
    namespace new_api
    {
        using namespace ndn;
        
        class RttEstimation;
        
        class BufferEstimator : public ndnlog::new_api::ILoggingObject
        {
        public:
            
            static const boost::int64_t MinBufferSizeMs;
            
            BufferEstimator():minBufferSizeMs_(MinBufferSizeMs){}
            BufferEstimator(const boost::shared_ptr<RttEstimation>& rttEstimation,
                            boost::int64_t minBufferSizeMs = MinBufferSizeMs);
            ~BufferEstimator(){}
            
            void
            setProducerRate(double producerRate) DEPRECATED;
            
            void
            setRttEstimation(const boost::shared_ptr<RttEstimation>& rttEstimation)
            { rttEstimation_ = rttEstimation; }
            
            void
            setMinimalBufferSize(boost::int64_t minimalBufferSize)
            { minBufferSizeMs_ = minimalBufferSize; }
            
            boost::int64_t
            getTargetSize();
            
        private:
            boost::shared_ptr<RttEstimation> rttEstimation_;
            boost::int64_t minBufferSizeMs_;
        };
    }
}

#endif /* defined(__ndnrtc__buffer_estimator__) */
