#pragma once
#include <h264/h264_stream.h>
#include "nal_parse.h"
namespace nal
{
    class H264NalParse : public NalParse
    {
    public:
        H264NalParse(const std::string& file_path);
        ~H264NalParse();
    private:
        virtual void ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit) override;
        virtual std::string NalTypeToString(int type) override;
        virtual std::string SliceTypeToString(int nal_type, int type) override;
    private:
        h264_stream_t* h264_stream_;
    };
}
