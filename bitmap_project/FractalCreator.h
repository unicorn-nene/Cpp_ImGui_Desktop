#ifndef FRACTALCREATOR_H_
#define FRACTALCREATOR_H_

#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <cmath>
#include "Zoom.h"
#include "Bitmap.h"
#include "ZoomList.h"
#include "RGB.h"

namespace neneofprogramming
{

    /**
     * @brief 负责生成分形图像（如 Mandelbrot 分形），并将结果写入 BMP 图像
     *
     *   该类是程序的核心控制器，负责：
     * - 管理缩放（ZoomList）
     * - 计算每个像素的迭代次数（Mandelbrot）
     * - 根据迭代次数生成颜色映射（使用 RGB 渐变）
     * - 将最终图像写入 BMP 文件
     */
    class FractalCreator
    {
    public:
        FractalCreator();
        FractalCreator(int width, int height);
        virtual ~FractalCreator();

        void run(std::string name);
        void addZoom(const Zoom &zoom);
        void addRange(double rangeEnd, const RGB &rgb);

    private:
        void calculateIteration();
        void calculateTotalIterations();
        void calculateRangeTotals();
        void drawFractal();
        void writeBitmap(std::string name);
        int getRange(int iterations) const;

    private:
        int width_{};                        // 图像宽度
        int height_{};                       // 图像高度
        int total_{0};                       // 总迭代像素数量
        std::unique_ptr<int[]> histogram_{}; // 直方图:记录每个迭代次数出现的次数
        std::unique_ptr<int[]> fractal_{};   // 储存每个像素的迭代次数的结果
        Bitmap bitmap_{};                    // 最终输出的 位图
        ZoomList zoomList_{};                // 缩放列表

        std::vector<int> ranges_{};      // 颜色区间上限(对应迭代次数)
        std::vector<RGB> colors_{};      // 每个区间的其实颜色
        std::vector<int> rangeTotals_{}; // 每个区间的像素总数

        bool isGotFirstRange_{false}; // 是否已添加第一个颜色区间
    };

} /* namespace neneofprogramming */

#endif /* FRACTALCREATOR_H_ */