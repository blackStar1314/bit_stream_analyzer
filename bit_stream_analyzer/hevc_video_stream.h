#pragma once
#include "video_stream_interface.h"
#include <string>
#include <memory>

namespace video_stream
{
    using namespace std::string_literals;
    class NalParse;
    class HevcVideoStream : public IVideoStream
    {
    public:
        HevcVideoStream(std::shared_ptr<NalParse> nal_parse);
        ~HevcVideoStream();
    public:
        virtual std::string GetName() override;
        virtual VideoFormat GetVideoFormat() override;
        virtual uint16_t GetPictureWidth() override;
        virtual uint16_t GetPicttureHeight() override;
        virtual uint8_t GetFps() override;
        virtual uint64_t GetTotalFrames() override;
        virtual bool Parse() override;
    private:
        std::shared_ptr<NalParse> nal_parse_;
    };
}
