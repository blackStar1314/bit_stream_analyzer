#include "hevc_nal_parse.h"

namespace nal
{
    HevcNalParse::HevcNalParse(const std::string& file_path)
        : NalParse(file_path)
    {
        hevc_stream_ = h265_new();
    }

    HevcNalParse::~HevcNalParse()
    {
        if (hevc_stream_)
        {
            h265_free(hevc_stream_);
            hevc_stream_ = nullptr;
        }
    }
    void HevcNalParse::ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit)
    {
        if (!hevc_stream_)
        {
            return;
        }

        auto cur_pos = in_file_stream_.tellg();
        Seek(nal_unit->offset + nal_unit->start_code.length() - 1);
        auto len = nal_unit->length - nal_unit->start_code.length() + 1;
        std::unique_ptr<uint8_t> buffer(new uint8_t[len]);

        in_file_stream_.read(reinterpret_cast<char*>(buffer.get()), sizeof(char) * len);
        auto size = in_file_stream_.gcount();
        if (size != len)
        {
            return;
        }

        h265_read_nal_unit(hevc_stream_, buffer.get(), len);

        nal_unit->type = NalTypeToString(hevc_stream_->nal->nal_unit_type);
        nal_unit->slice_type = SliceTypeToString(hevc_stream_->nal->nal_unit_type, hevc_stream_->sh->slice_type);

        in_file_stream_.seekg(cur_pos);
    }
    std::string HevcNalParse::NalTypeToString(int type)
    {
        std::string des("Unknown");
        static std::map<int, std::string> s_des =
        {
            {NAL_UNIT_CODED_SLICE_TRAIL_N, "Coded slice segment of a non-TSA, non-STSA trailing picture"},
            {NAL_UNIT_CODED_SLICE_TRAIL_R, "Coded slice segment of a non-TSA, non-STSA trailing picture"},
            {NAL_UNIT_CODED_SLICE_TSA_N, "Coded slice segment of a TSA picture"},
            {NAL_UNIT_CODED_SLICE_TSA_R, "Coded slice segment of a TSA picture"},
            {NAL_UNIT_CODED_SLICE_RADL_N, "Coded slice segment of a TSA picture"},
            {NAL_UNIT_CODED_SLICE_RADL_R, "Coded slice segment of a TSA picture"},
            {NAL_UNIT_CODED_SLICE_IDR_W_RADL, "Coded slice of an IDR picture"},
            {NAL_UNIT_CODED_SLICE_IDR_N_LP, "Coded slice of an IDR picture"},
            {NAL_UNIT_CODED_SLICE_CRA, "Coded slice segment of a CRA picture"},
            {NAL_UNIT_PREFIX_SEI, "Supplemental enhancement information"},
            {NAL_UNIT_SUFFIX_SEI, "Supplemental enhancement information"},
            {NAL_UNIT_VPS, "Video parameter set"},
            {NAL_UNIT_SPS, "Sequence parameter set"},
            {NAL_UNIT_PPS, "Picture parameter set"},
            {NAL_UNIT_AUD, "Access UD"},
            {NAL_UNIT_EOS, "END_SEQUENCE"},
            {NAL_UNIT_EOB, "END_STREAM"},
            {NAL_UNIT_FILLER_DATA, "FILLER_DATA"},
        };

        auto iter = s_des.find(type);
        if (iter != s_des.end())
        {
            des = iter->second;
        }

        return des;
    }
    std::string HevcNalParse::SliceTypeToString(int nal_type, int type)
    {
        std::string des;
        bool is_des = false;

        switch (nal_type)
        {
        case NAL_UNIT_CODED_SLICE_TRAIL_N:
        case NAL_UNIT_CODED_SLICE_TRAIL_R:
        case NAL_UNIT_CODED_SLICE_TSA_N:
        case NAL_UNIT_CODED_SLICE_TSA_R:
        case NAL_UNIT_CODED_SLICE_RADL_N:
        case NAL_UNIT_CODED_SLICE_RADL_R:
            is_des = true;
            break;
        case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
        case NAL_UNIT_CODED_SLICE_IDR_N_LP:
            des = "IDR";
            break;
        case NAL_UNIT_CODED_SLICE_CRA:
            des = "CRA";
            break;
        case NAL_UNIT_PREFIX_SEI:
        case NAL_UNIT_SUFFIX_SEI:
            des = "SEI";
            break;
        case NAL_UNIT_VPS:
            des = "VPS";
            break;
        case NAL_UNIT_SPS:
            des = "SPS";
            break;
        case NAL_UNIT_PPS:
            des = "PPS";
            break;
        case NAL_UNIT_AUD:
            des = "AUD";
            break;
        default:
            break;
        }

        if (is_des)
        {
            static std::map<int, std::string> s_des =
            {
                {H265_SH_SLICE_TYPE_B, "B Slice"},
                {H265_SH_SLICE_TYPE_P, "P Slice"},
                {H265_SH_SLICE_TYPE_I, "I Slice"},
            };

            auto iter = s_des.find(type);

            if (iter != s_des.end())
            {
                des = iter->second;
            }
        }

        return des;
    }
}
