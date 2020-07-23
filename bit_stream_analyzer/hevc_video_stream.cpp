#include "hevc_video_stream.h"

namespace video_stream
{
    HevcVideoStream::HevcVideoStream(std::shared_ptr<NalParse> nal_parse)
        : nal_parse_(nal_parse)
    {
    }

    HevcVideoStream::~HevcVideoStream() = default;

    std::string HevcVideoStream::GetName()
    {
        return "HEVC"s;
    }


}
