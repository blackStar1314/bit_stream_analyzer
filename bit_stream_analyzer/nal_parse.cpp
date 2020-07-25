#include "nal_parse.h"
#include <experimental/filesystem>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <iostream>

namespace nal
{
    namespace fs = std::experimental::filesystem;
    using namespace std::string_literals;

    NalParse::NalParse() = default;

    NalParse::NalParse(const std::string& file_path) noexcept
        :file_path_(file_path)
    {
        in_file_stream_.open(file_path, std::ios::binary);
    }

    NalParse::~NalParse()
    {
        if (in_file_stream_.is_open())
        {
            in_file_stream_.close();
        }
    }

    DecodeFormat NalParse::GetStreamType(const std::string& file_path)
    {
        std::ifstream in_file(file_path, std::ios::binary);
        if (!in_file)
        {
            return DecodeFormat::NONE;
        }

        fs::path path = fs::u8path(file_path);
        if (!path.has_extension())
        {
            // TODO: 暂时不考虑无扩展名称的
            return DecodeFormat::NONE;
        }
        auto extension = path.extension().u8string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        static std::map<std::string, DecodeFormat> s_type =
        {
            {".264"s, DecodeFormat::H264},
            {".hevc"s, DecodeFormat::HEVC},
            {".265"s,DecodeFormat::HEVC}
        };

        auto iter = s_type.find(extension);

        if (iter != s_type.end())
        {
            return iter->second;
        }

        return DecodeFormat::NONE;

    }
    bool NalParse::ProbeNal()
    {
        if (!in_file_stream_.is_open())
        {
            return false;
        }
        Seek(0);
        return Probe();
    }
    std::map<uint64_t, std::shared_ptr<NalUnit>> NalParse::GetNals() const
    {
        return nal_;
    }
    std::string NalParse::GetNalData(uint64_t no)
    {
        return ""s;
        /*  std::advance(nal_, no);
          auto iter = nal_.begin();
          if (iter == nal_.end())
          {
              return ""s;
          }

          return ParseNalData(iter->second);*/
    }
    bool NalParse::ReOpenFile()
    {
        if (in_file_stream_.is_open())
        {
            in_file_stream_.close();
            in_file_stream_.open(file_path_, std::ios::binary);
        }
        return in_file_stream_.is_open();
    }
    std::string NalParse::ParseNalData(std::shared_ptr<NalUnit> nal_unit)
    {
        Seek(nal_unit->offset);

        auto size = nal_unit->length;
        std::unique_ptr<uint8_t> buffer(new uint8_t[size]);
        in_file_stream_.read(reinterpret_cast<char*>(buffer.get()), size);
        std::string data(reinterpret_cast<char*>(buffer.get()), size);
        return data;
    }
    bool NalParse::Probe()
    {
        uint64_t id = 0;
        int offset = FindFirstNalUnitOffset(in_file_stream_);
        if (offset < 0)
        {
            return false;
        }

        Seek(offset);

        while (!is_eof_)
        {
            auto nal_unit = GetAnnexBNalUnit(in_file_stream_);
            if (!nal_unit)
            {
                break;
            }
            if (nal_unit)
            {
                nal_.emplace(id++, nal_unit);
            }
        }

        return nal_.size() > 0;
    }
    void NalParse::Seek(uint64_t pos)
    {
        in_file_stream_.seekg(std::ifstream::beg);
        in_file_stream_.seekg(pos);
    }
    std::shared_ptr<NalUnit> NalParse::GetAnnexBNalUnit(std::ifstream& in_f_stream)
    {
        std::shared_ptr<NalUnit> nal;
        auto start_pos = in_file_stream_.tellg();
        auto start_code = FindStartCode(in_file_stream_);
        if (start_code.empty())
        {
            return nal;
        }

        nal.reset(new NalUnit);

        nal->offset = static_cast<int>(start_pos);
        start_code_len_ = start_code.size() - 1;
        for (auto i = 0; i < start_code_len_; ++i)
        {
            PushBack(i, start_code[i]);
        }

        int offset = 0;
        auto next_start_code = FindNextStartCode(in_file_stream_, offset);
        if (in_file_stream_.eof())
        {
            // 如果文件流指针文件结束后，不能通过seek移动到前面的位置，这里需要重新关闭打开下
            is_eof_ = true;
            offset += 1 + start_code_len_;
            ReOpenFile();
        }
        else
        {
            offset += 1;
        }
        auto nal_unit_len = offset;
        in_file_stream_.seekg(static_cast<int>(start_pos) + nal_unit_len);

        nal->length = static_cast<int>(nal_unit_len);

        nal->start_code = start_code;
        ReadNalUnit(nal);
        return nal;
    }

    void NalParse::ReadNalUnit(std::shared_ptr<NalUnit>& nal_unit)
    {
    }

    int NalParse::FindFirstNalUnitOffset(std::ifstream& in_f_stream)
    {
        auto start_pos = in_file_stream_.tellg();
        auto start_code = FindStartCode(in_file_stream_);
        if (start_code.empty())
        {
            return -1;
        }

        auto end_pos = in_file_stream_.tellg();
        int offset = static_cast<int>((end_pos - start_pos) - start_code.length());

        return offset;
    }

    std::string NalParse::FindStartCode(std::ifstream& in_f_stream)
    {
        // 这里文件流指针一定指向了开始码的首地址
        auto prev_pos = in_file_stream_.tellg();
        char buffer[5]{ 0 };
        in_file_stream_.read(buffer, sizeof(char) * 5);
        auto size = in_file_stream_.gcount();
        if (size != 4 && size != 5)
        {
            return ""s;
        }

        int start_code_len = 0;
        if (size == 4)
        {
            FindStartCode3(reinterpret_cast<uint8_t*>(buffer));
            start_code_len = size;
        }
        else if (size == 5)
        {
            FindStartCode4(reinterpret_cast<uint8_t*>(buffer));
            start_code_len = size;
        }

        if (0 == start_code_len) return ""s;

        auto start_code = std::string(buffer, sizeof(buffer)); // 这样处理就不会被截断
        if (start_code_len == 4)
        {
            start_code.pop_back();
        }
        auto cur_pos = static_cast<int>(prev_pos) + start_code_len;
        in_file_stream_.seekg(cur_pos);

        return start_code;
    }

    /**
     * .查找下一个start code. 相邻的NAL Unit 由于类型不同，可能开始码也会发生变化.
     *  然后找到一个开始码后需要记录下来，下一个开始前的数据需要一个字节一个字节的移动，比较，没有规律可言.
     *
     * \param in_f_stream : 文件流，全局的，控制文件流的位置
     * \param len : 开始码直接的数据大小
     * \return : 返回 开始码字 + 之后的一个字节数据的字符串
     */
    std::string NalParse::FindNextStartCode(std::ifstream& in_f_stream, int& len)
    {
        char data[1]{ 0 };
        bool find_start_code = false;
        int move_offset = 0;
        while (!in_file_stream_.eof())
        {
            in_file_stream_.read(data, sizeof(char) * 1);
            move_offset += in_file_stream_.gcount();
            UpdateStartCodeBufferValue(data[0]);

            // 按照实际相邻开始码之间数据个数 >= 0 找到下一个开始码必须要移动上一个开始码的大小
            if (move_offset >= start_code_len_ && FindStartCode3(reinterpret_cast<uint8_t*>(prev_start_code_buffer_)))
            {
                start_code_len_ = 3;
                find_start_code = true;
                break;
            }

            if (move_offset >= start_code_len_ && FindStartCode4(reinterpret_cast<uint8_t*>(prev_start_code_buffer_)))
            {
                start_code_len_ = 4;
                find_start_code = true;
                break;
            }
        }

        std::string start_code;

        if (find_start_code)
        {
            if (data[0] == 1)
            {
                auto prev_pos = in_file_stream_.tellg();
                in_file_stream_.read(data, sizeof(char) * 1);
                PushBack(start_code_len_, data[0]);
            }
            start_code.resize(start_code_len_ + 1);
            for (auto i = 0; i < start_code_len_ + 1; ++i)
            {
                start_code[i] = prev_start_code_buffer_[i];
            }
        }

        len = move_offset;

        return start_code;
    }
    std::string NalParse::NalTypeToString(int type)
    {
        return ""s;
    }
    std::string NalParse::SliceTypeToString(int nal_type, int type)
    {
        return ""s;
    }
}
