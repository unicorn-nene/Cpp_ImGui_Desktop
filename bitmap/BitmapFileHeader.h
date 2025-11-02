#ifndef BITMAPFILEHEADER_H_
#define BITMAPFILEHEADER_H_
#include <cstdint>

using namespace std;

#pragma pack(2) // 按照 2 字节对齐, 确保结构体与BMP文件格式的字节布局一致
namespace neneofprogramming
{
    /**
     * @brief 表示BMP文件的文件头结构
     *
     * BMP文件的开头是一个14字节的文件头，用于描述文件的基本信息，
     * 包括标识符、文件大小、数据起始偏移等。该结构体采用2字节对齐，
     * 以确保其内存布局与BMP文件格式严格对应
     *
     */
    struct BitmapFileHeader
    {
        char header_[2]{'B', 'M'}; // 文件类型标识符
        int32_t fileSize_{};       // 整个BMP文件大小
        int32_t reserved_{0};      // 保留字段
        int32_t dataOffset_{};     // 像素数据相对于文件起始位置的偏移量
    };

} /* namespace neneofprogramming */

#endif /* BITMAPFILEHEADER_H_ */