#pragma once
#include <memory>
#include <map>
#include <string>
#include <fstream>

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
        NalParse(const std::string& file_path);
        virtual ~NalParse();
    public:
        virtual bool ParseNal();
    protected:
        std::map<uint64_t, std::shared_ptr<NalUnit>> nal_;
        std::ifstream in_file_stream_;
    };
}
