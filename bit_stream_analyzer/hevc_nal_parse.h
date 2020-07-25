#pragma once
#include <hevc/hevc_stream.h>
#include "nal_parse.h"

namespace nal
{
    class HevcNalParse : public NalParse
    {
    public:
        explicit HevcNalParse(const std::string& file_path);
        ~HevcNalParse();
    private:
        virtual void ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit) override;
        virtual std::string NalTypeToString(int type) override;
        virtual std::string SliceTypeToString(int nal_type, int type) override;
    private:
        typedef h265_stream_t hevc_stream_t;
        hevc_stream_t* hevc_stream_;
    };
}
