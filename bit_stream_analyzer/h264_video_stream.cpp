#include "h264_video_stream.h"

namespace video_stream
{
    H264VideoStream::H264VideoStream(std::shared_ptr<NalParse> nal_parse)
        : nal_parse_(nal_parse)
    {
    }

    H264VideoStream::~H264VideoStream() = default;
    std::string H264VideoStream::GetName()
    {
        return "H264"s;
    }

    VideoFormat H264VideoStream::GetVideoFormat()
    {
        return VideoFormat();
    }


}
