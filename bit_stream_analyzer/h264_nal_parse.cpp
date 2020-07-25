#include "h264_nal_parse.h"

namespace nal
{
    H264NalParse::H264NalParse(const std::string& file_path)
        : NalParse(file_path)
    {
        h264_stream_ = h264_new();
    }

    H264NalParse::~H264NalParse()
    {
        if (h264_stream_)
        {
            h264_free(h264_stream_);
            h264_stream_ = nullptr;
        }
    }
    void H264NalParse::ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit)
    {
        if (!h264_stream_)
        {
            return;
        }
        auto cur_pos = in_file_stream_.tellg();
        Seek(nal_unit->offset + nal_unit->start_code.length() - 1);
        auto seek_pos = in_file_stream_.tellg();
        auto len = nal_unit->length - nal_unit->start_code.length() + 1;
        std::unique_ptr<uint8_t> buffer(new uint8_t[len]);

        in_file_stream_.read(reinterpret_cast<char*>(buffer.get()), sizeof(char) * len);
        auto size = in_file_stream_.gcount();
        if (size != len)
        {
            return;
        }

        read_nal_unit(h264_stream_, buffer.get(), len);

        nal_unit->type = NalTypeToString(h264_stream_->nal->nal_unit_type);
        nal_unit->slice_type = SliceTypeToString(h264_stream_->nal->nal_unit_type, h264_stream_->sh->slice_type);

        in_file_stream_.seekg(cur_pos);
    }
    std::string H264NalParse::NalTypeToString(int type)
    {
        std::string des("Other");
        static std::map<int, std::string> s_des =
        {
            {0, "Unspecified"},
            {1, "Coded slice of a non-IDR picture"},
            {2, "DPA"},
            {3, "DPB"},
            {4, "DPC"},
            {5, "Coded slice of an IDR picture"},
            {6, "Supplemental enhancement information"},
            {7, "Sequence parameter set"},
            {8, "Picture parameter set"},
            {9, "Access UD"},
            {10, "END_SEQUENCE"},
            {11, "END_STREAM"},
            {12, "FILLER_DATA"},
            {13, "SPS_EXT"},
            {19, "AUXILIARY_SLICE"},
        };

        auto iter = s_des.find(type);
        if (iter != s_des.end())
        {
            des = iter->second;
        }

        return des;
    }
    std::string H264NalParse::SliceTypeToString(int nal_type, int type)
    {
        std::string des;
        bool is_des = false;

        switch (nal_type)
        {
        case 1:
            is_des = true;
            break;
        case 5:
            des = "IDR";
            break;
        case 6:
            des = "SEI";
            break;
        case 7:
            des = "SPS";
            break;
        case 8:
            des = "PPS";
            break;
        case 9:
            des = "AUO";
            break;
        default:
            break;
        }

        if (is_des)
        {
            static std::map<int, std::string> s_des =
            {
                {0, "P Slice"},
                {1, "B Slice"},
                {2, "I Slice"},
                {3, "DPB"},
                {4, "DPC"},
                {5, "P Slice"},
                {6, "B Slice"},
                {7, "I Slice"},
                {8, "Picture parameter set"},
                {9, "Access UD"},
                {10, "END_SEQUENCE"},
                {11, "END_STREAM"},
                {12, "FILLER_DATA"},
                {13, "SPS_EXT"},
                {19, "AUXILIARY_SLICE"},
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
