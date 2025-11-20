#include <fstream>
#include "Bitmap.h"
#include "BitmapFileHeader.h"
#include "BitmapInfoHeader.h"

using namespace neneofprogramming;
using namespace std;

namespace neneofprogramming
{
    Bitmap::Bitmap()
        : width_(0), height_(0), pPixels_(nullptr)
    {
    }

    /**
     * @brief Construct a new Bitmap:: Bitmap object
     *
     * @param width
     * @param height
     */
    Bitmap::Bitmap(int width, int height)
        : width_(width), height_(height), pPixels_(new uint8_t[width * height * 3]{})
    {
    }

    /**
     * @brief 设定指定坐标(x, y)像素的 RGB 值
     *
     * @param x      像素的水平坐标（从左到右）
     * @param y      像素的垂直坐标（从上到下）
     * @param red    红色分量（0 ~ 255）
     * @param green  绿色分量（0 ~ 255）
     * @param blue   蓝色分量（0 ~ 255）
     */
    void Bitmap::setPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
    {
        uint8_t *pPixel = pPixels_.get();

        pPixel += (y * 3 * width_) + (x * 3);

        // BMP 格式中颜色顺序为 BGR（而非 RGB）
        pPixel[0] = blue;
        pPixel[1] = green;
        pPixel[2] = red;
    }

    /**
     * @brief
     *
     * @param filename
     * @return true
     * @return false
     */
    bool Bitmap::write(string filename)
    {
        BitmapFileHeader fileHeader;
        BitmapInfoHeader infoHeader;

        fileHeader.fileSize_ = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + width_ * height_ * 3 * sizeof(uint8_t); // sizeof(uint8_t) = 1
        fileHeader.dataOffset_ = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

        infoHeader.width_ = width_;
        infoHeader.height_ = height_;

        ofstream file;
        file.open(filename, ios::out | ios::binary);

        if (!file)
            return false;

        file.write(reinterpret_cast<const char *>(&fileHeader), sizeof(fileHeader));
        file.write(reinterpret_cast<const char *>(&infoHeader), sizeof(infoHeader));
        file.write(reinterpret_cast<const char *>(pPixels_.get()), width_ * height_ * 3);

        file.close();

        return true;
    }

    Bitmap::~Bitmap()
    {
    }
}