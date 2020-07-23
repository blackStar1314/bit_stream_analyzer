#pragma once
#include "video_stream_interface.h"
#include <string>
#include <memory>
#include <map>

namespace video_stream
{
    using namespace std::string_literals;
    class NalParse;
    class H264VideoStream : public IVideoStream
    {
    public:
        H264VideoStream(std::shared_ptr<NalParse> nal_parse);
        ~H264VideoStream();
    public:
        virtual std::string GetName() override;
        virtual VideoFormat GetVideoFormat() override;
        virtual uint16_t GetPictureWidth() override;
        virtual uint16_t GetPicttureHeight() override;
        virtual uint8_t GetFps() override;
        virtual uint64_t GetTotalFrames() override;
        virtual bool Parse() override;
    public:
    private:
        std::shared_ptr<NalParse> nal_parse_;
    };
}
