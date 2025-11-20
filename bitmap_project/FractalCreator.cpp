#include <iostream>
#include <cassert>
#include "FractalCreator.h"
#include "Mandelbrot.h"
#include "RGB.h"

namespace neneofprogramming
{

    FractalCreator::FractalCreator()
    {
    }

    FractalCreator::FractalCreator(int width, int height)
        : width_(width), height_(height),
          histogram_(new int[Mandelbrot::MAX_ITETATIONS + 1]{0}),
          fractal_(new int[width_ * height_]{0}),
          bitmap_(width_, height_),
          zoomList_(width_, height_)
    {
        // 添加一次全局缩放,使得分形图案初始居中
        zoomList_.add(Zoom(width_ / 2, height_ / 2, 4.0 / width_));
    }

    FractalCreator::~FractalCreator()
    {
    }

    /**
     * @brief 执行整个分形程序流程
     *
     * @param name 要输出的BMP文件名
     */
    void FractalCreator::run(std::string name)
    {
        calculateIteration();       // 计算迭代次数
        calculateTotalIterations(); // 统计迭代次数
        calculateRangeTotals();     // 计算颜色区间
        drawFractal();              // 绘制分型图像
        writeBitmap(name);          // 写入 BMP 文件
    }

    /**
     * @brief 计算每个像素的迭代次数，并统计到直方图 histogram_
     *
     */
    void FractalCreator::calculateIteration()
    {
        for (int y = 0; y < height_; y++)
        {
            for (int x = 0; x < width_; x++)
            {
                // 像素坐标 ---→ 分形（Mandelbrot）平面坐标
                std::pair<double, double> coords = zoomList_.doZoom(x, y);

                // 获取当前点的迭代次数
                int iterations = Mandelbrot::getIterations(coords.first, coords.second);

                fractal_[y * width_ + x] = iterations;

                if (iterations != Mandelbrot::MAX_ITETATIONS)
                    histogram_[iterations]++;
            }
        }
    }

    /**
     * @brief 统计所有像素的迭代次数总和
     *
     */
    void FractalCreator::calculateTotalIterations()
    {

        for (int i = 0; i < Mandelbrot::MAX_ITETATIONS; ++i)
        {
            total_ += histogram_[i];
        }

        std::cout << "Overall total2: " << total_ << std::endl;
    }

    /**
     * @brief 计算每个颜色区间的总像素数
     *
     */
    void FractalCreator::calculateRangeTotals()
    {
        int rangeIndex = 0;

        for (int i = 0; i < Mandelbrot::MAX_ITETATIONS; i++)
        {
            int pixels = histogram_[i];

            if (i >= ranges_[rangeIndex + 1])
                rangeIndex++;

            rangeTotals_[rangeIndex] += pixels;
        }

        int overallTotal = 0;
        for (int value : rangeTotals_)
        {
            std::cout << "Range total: " << value << std::endl;
            overallTotal += value;
        }

        std::cout << "Overall total1: " << overallTotal << std::endl;
    }

    /**
     * @brief 根据迭代次数绘制分形图像（着色）
     *
     */
    void FractalCreator::drawFractal()
    {

        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                int iterations = fractal_[y * width_ + x];

                int range = getRange(iterations);
                int rangeTotal = rangeTotals_[range];
                int rangeStart = ranges_[range];

                RGB &startColor = colors_[range];
                RGB &endColor = colors_[range + 1];
                RGB colorDiff = endColor - startColor;

                uint8_t red = 0;
                uint8_t green = 0;
                uint8_t blue = 0;

                if (iterations != Mandelbrot::MAX_ITETATIONS)
                {
                    // 累加当前区间的像素数量
                    int totalPixels = 0;

                    for (int i = rangeStart; i <= iterations; ++i)
                    {
                        totalPixels += histogram_[i];
                    }

                    // 根据比例计算渐变颜色
                    red = startColor.r_ + colorDiff.r_ * static_cast<double>(totalPixels) / rangeTotal;
                    green = startColor.g_ + colorDiff.g_ * static_cast<double>(totalPixels) / rangeTotal;
                    blue = startColor.b_ + colorDiff.b_ * static_cast<double>(totalPixels) / rangeTotal;
                }

                bitmap_.setPixel(x, y, red, green, blue);
            }
        }
    }

    /**
     * @brief 添加一次缩放操作
     * @param zoom 缩放中心与缩放比例
     *
     * 多次缩放叠加可用于聚焦不同区域
     */
    void FractalCreator::addZoom(const Zoom &zoom)
    {
        zoomList_.add(zoom);
    }

    /**
     * @brief 将生成的图像写入 BMP 文件
     * @param name 输出文件名
     */
    void FractalCreator::writeBitmap(std::string name)
    {
        bitmap_.write(name);
    }

    /**
     * @brief 添加颜色区间
     * @param rangeEnd 区间结束比例（0~1之间的小数）
     * @param rgb 区间对应颜色。
     *
     * 每个颜色区间对应分形迭代值的一部分，用于颜色渐变。
     */
    void FractalCreator::addRange(double rangeEnd, const RGB &rgb)
    {
        ranges_.push_back(rangeEnd * Mandelbrot::MAX_ITETATIONS);
        colors_.push_back(rgb);

        if (isGotFirstRange_)
        {
            rangeTotals_.push_back(0);
        }

        isGotFirstRange_ = true;
    }

    /**
     * @brief 根据迭代次数判断该像素属于哪个颜色区间。
     * @param iterations 该像素的迭代次数。
     * @return 区间索引（ranges_ 中的下标）。
     */
    int FractalCreator::getRange(int iterations) const
    {
        int range = 0;

        for (int i = 1; i < ranges_.size(); i++)
        {
            range = i;

            if (ranges_[i] > iterations)
            {
                break;
            }
        }

        range--;

        assert(range > -1);
        assert(range < ranges_.size());

        return range;
    }

} /* namespace neneofprogramming */