#ifndef BITMAPINFOHEADER_H_
#define BITMAPINFOHEADER_H_
#include <cstdint>

using namespace std;

#pragma pack(2)

namespace neneofprogramming
{

    /**
     * @brief 表示BMP文件的信息头结构
     *
     * 该结构体紧随在文件头（BitmapFileHeader）之后，长度通常为40字节。
     * 它描述了位图的详细信息，如宽高、色深、压缩方式等，
     * 是解析BMP图像数据时的重要元数据部分
     */
    struct BitmapInfoHeader
    {
        int32_t headerSize_{40};             // 文件信息头的大小(标准位图格式固定为40)
        int32_t width_{};                    // 图像的宽度(像素)
        int32_t height_{};                   // 图像的高度(像素)
        int16_t planes_{1};                  // 颜色平面数(BMP要求为 1)
        int16_t bitsPerPixel_{24};           // 每个像素的位数,即色深(24 = 3 * 8位)
        int32_t compression_{0};             // 压缩方式(0表示不压缩)
        int32_t dataSize_{0};                // 图像数据区大小(字节)
        int32_t horizontalResolution_{2400}; // 水平分辨率(像素/米)
        int32_t verticalResolution_{2400};   // 垂直分辨率(像素/米)
        int32_t colors_{0};                  // 调色板中的颜色数目,0 表示所有可能颜色
        int32_t importantColors_{0};         // 调色板中重要颜色数,0 表示所有可能颜色
    };
} /* namespace neneofprogramming */

#endif