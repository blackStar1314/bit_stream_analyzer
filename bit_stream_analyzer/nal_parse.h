#pragma once
#include <memory>
#include <map>
#include <string>
#include <fstream>
#include <atomic>
#include "common.h"

namespace nal
{
    struct NalUnit
    {
        std::string type;           // NAL类型
        int offset;                // 帧的偏移值
        int length;                // 帧的数据大小
        std::string start_code;    // 开始码 打包方式:Annex-B
        std::string slice_type;    // 帧类型
    };
    class NalParse
    {
    public:
        NalParse();
        NalParse(const std::string& file_path) noexcept;
        virtual ~NalParse();
    public:
        static DecodeFormat GetStreamType(const std::string& file_path);
        bool ProbeNal();
        std::map<uint64_t, std::shared_ptr<NalUnit>> GetNals() const;
        std::string GetNalData(uint64_t no);
        bool ReOpenFile();
    protected:
        virtual std::string ParseNalData(std::shared_ptr<NalUnit> nal_unit);
        virtual bool Probe();
        void Seek(uint64_t pos);
        std::shared_ptr<NalUnit> GetAnnexBNalUnit(std::ifstream& in_f_stream);
        virtual void ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit);
    private:
        int FindFirstNalUnitOffset(std::ifstream& in_f_stream);
        std::string FindStartCode(std::ifstream& in_f_stream);
        std::string FindNextStartCode(std::ifstream& in_f_stream, int& len);

        inline bool FindStartCode3(const uint8_t* buffer)
        {
            return (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 1);
        }

        inline bool FindStartCode4(const uint8_t* buffer)
        {
            return (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 1);
        }
    protected:
        virtual std::string NalTypeToString(int type);
        virtual std::string SliceTypeToString(int nal_type, int type);

    private:

        void UpdateStartCodeBufferValue(const char ch)
        {
            memcpy(prev_start_code_buffer_, prev_start_code_buffer_ + 1, start_code_len_ - 1);
            prev_start_code_buffer_[start_code_len_ - 1] = ch;

        }

        void PushBack(int pos, const char ch)
        {
            prev_start_code_buffer_[pos] = ch;
        }

    protected:
        std::map<uint64_t, std::shared_ptr<NalUnit>> nal_;
        std::string file_path_;
        std::ifstream in_file_stream_;
        char prev_start_code_buffer_[5]{ 0 };
        size_t start_code_len_ = 0;
    };
}
