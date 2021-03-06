//
//  video-renderer.h
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#ifndef __ndnrtc__video_renderer__
#define __ndnrtc__video_renderer__

#include <modules/video_render/video_render_impl.h>

#include "renderer.h"
#include "ndnrtc-common.h"
#include "base-capturer.h"
#include "interfaces.h"

namespace ndnrtc
{
    class IVideoRenderer : public new_api::IRenderer, public IRawFrameConsumer,
    public new_api::NdnRtcComponent
    {
    public:
        IVideoRenderer(){}
        virtual ~IVideoRenderer(){};
    };
    
    /**
     * This class is used for sending RGB buffers to the external renderer. 
     * It conforms to the internal library interfaces (i.e. IRawFrameConsumer) 
     * and can be used as a normal renderer. However, internally it does not 
     * perform actual rendering - instead, it performs transcoding incoming 
     * frames into RGB format and copies RGB data into buffer, provided by 
     * external renderer.
     */
    class ExternalVideoRendererAdaptor : public IVideoRenderer
    {
    public:
        ExternalVideoRendererAdaptor(IExternalRenderer* externalRenderer);
        virtual ~ExternalVideoRendererAdaptor(){}
        
        int
        init();
        
        int
        startRendering(const std::string &windowName = "Renderer");
        
        int
        stopRendering();
        
        void
        onDeliverFrame(WebRtcVideoFrame &frame, double timestamp);
        
    private:
        IExternalRenderer* externalRenderer_;
    };
}


#endif /* defined(__ndnrtc__video_renderer__) */
