//
//  playout.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//  Created: 3/19/14
//

#ifndef __ndnrtc__playout__
#define __ndnrtc__playout__

#include "ndnrtc-common.h"
#include "jitter-timing.h"
#include "consumer.h"
#include "frame-buffer.h"
#include "statistics.h"

namespace ndnrtc{
    namespace new_api {
        
        /**
         * Base class for playout mechanisms. The core playout logic is similar 
         * for audio and video streams. Differences must be implemented in 
         * overriden method playbackPacket which is called from main playout 
         * routine each time playout timer fires. Necessary information is 
         * provided as arguments to the method.
         */
        class Playout : public NdnRtcComponent, public statistics::StatObject
        {
        public:
            static const int BufferCheckInterval;
            
            Playout(Consumer* consumer,
                    const boost::shared_ptr<statistics::StatisticsStorage>& statStorage);
            virtual ~Playout();
            
            virtual int
            init(void* frameConsumer);
            
            virtual int
            start(int initialAdjustment = 0);
            
            virtual int
            stop();
            
            void
            setLogger(ndnlog::new_api::Logger* logger);
            
            void
            setDescription(const std::string& desc);
            
            bool
            isRunning()
            { return isRunning_; }
            
            void
            setPlaybackAdjustment(int playbackAdjustment)
            { playbackAdjustment_ = playbackAdjustment; }
            
        protected:
            bool isRunning_;
            
            bool isInferredPlayback_;
            boost::int64_t lastPacketTs_;
            unsigned int inferredDelay_;
            int playbackAdjustment_;
            boost::int64_t bufferCheckTs_;
            
            Consumer* consumer_;
            boost::shared_ptr<FrameBuffer> frameBuffer_;
            
            boost::shared_ptr<JitterTiming> jitterTiming_;
            boost::mutex playoutMutex_;
            
            void* frameConsumer_;
            PacketData *data_;
            
            /**
             * This method should be overriden by derived classes for 
             * media-specific playback (audio/video)
             * @param packetTsLocal Packet local timestamp
             * @param data Packet data retrieved from the buffer
             * @param packetNo Packet playback number provided by a producer
             * @param isKey Indicates, whether the packet is a key frame (@note 
             * always false for audio packets)
             * @param assembledLevel Ratio indicating assembled level of the 
             * packet: number of fetched segments divided by total number of 
             * segments for this packet
             */
            virtual bool
            playbackPacket(boost::int64_t packetTsLocal, PacketData* data,
                           PacketNumber playbackPacketNo,
                           PacketNumber sequencePacketNo,
                           PacketNumber pairedPacketNo,
                           bool isKey, double assembledLevel) = 0;
            
            void
            updatePlaybackAdjustment();
            
            int
            playbackDelayAdjustment(int playbackDelay);
            
            int
            avSyncAdjustment(boost::int64_t nowTimestamp, int playbackDelay);
            
            bool
            processPlayout();
            
            void
            checkBuffer();
        };
    }
}

#endif /* defined(__ndnrtc__playout__) */
