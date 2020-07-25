// bit_stream_analyzer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "nal_parse.h"
#include "hevc_nal_parse.h"
#include "h264_nal_parse.h"
#include <iomanip>

using namespace nal;

int main()
{
    const std::string file_path = "test_res\\cap_del.265";
    auto type = NalParse::GetStreamType(file_path);
    std::cout << "stream type: " << type << std::endl;
    std::shared_ptr<NalParse> nal_parse;
    if (DecodeFormat::H264 == type)
    {
        nal_parse = std::make_shared<H264NalParse>(file_path);
    }
    if (DecodeFormat::HEVC == type)
    {
        nal_parse = std::make_shared<HevcNalParse>(file_path);
    }

    if (nal_parse->ProbeNal())
    {
        auto nals = nal_parse->GetNals();
        std::cerr << "No." << std::setw(20) << "Offset" << std::setw(40) << "Length" << std::setw(60) << "NAL Type" << std::setw(100) << "Info" << std::endl;
        for (auto iter = nals.begin(); iter != nals.end(); ++iter)
        {
            std::cerr << iter->first << std::setw(20) << std::hex << iter->second->offset << std::setw(40) << std::dec << iter->second->length
                << std::setw(60) << std::dec << iter->second->type << std::setw(100) << iter->second->slice_type << std::endl;
        }

        system("pause");
    }

    system("pause");
    return 0;
}
