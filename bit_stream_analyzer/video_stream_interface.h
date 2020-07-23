#pragma once
#include <cstdint>

namespace video_stream
{
    enum DecodeFormat
    {
        NONE = -1,
        H264,
        HEVC,
    };

    enum VideoFormat
    {
        RGB = 0,
        YUV420P,
    };


    class IVideoStream
    {
    public:
        virtual DecodeFormat GetDecodeFormat() = 0;
        virtual VideoFormat GetVideoFormat() = 0;
        virtual uint16_t GetPictureWidth() = 0;
        virtual uint16_t GetPicttureHeight() = 0;
        virtual uint8_t GetFps() = 0;
        virtual uint64_t GetTotalFrames() = 0;
        virtual bool Parse() = 0;
        virtual ~IVideoStream() = default;
    };
}
