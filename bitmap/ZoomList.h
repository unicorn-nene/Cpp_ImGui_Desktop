#ifndef ZOOMLIST_H_
#define ZOOMLIST_H_

#include <vector>
#include <utility>
#include "Zoom.h"

namespace neneofprogramming
{
    /**
     * @brief 管理一组缩放操作（Zoom），用于计算屏幕坐标到分形坐标的映射关系
     *
     * ZoomList 通过保存多个 Zoom（缩放操作），
     * 可以根据连续的放大操作计算出新的 Mandelbrot 平面坐标范围
     */
    class ZoomList
    {
    private:
        double xCenter_{0}; // 当前视图中心点的 分形平面坐标 X
        double yCenter_{0}; // 当前视图中心点的 分形平面坐标 Y
        double scale_{1.0}; // 当前缩放比例

        int width_{0};              // 图像宽度
        int height_{0};             // 图像高度
        std::vector<Zoom> zooms_{}; // 保存所有缩放操作的列表

    public:
        ZoomList();
        ZoomList(int width, int height);
        void add(const Zoom &zoom);
        std::pair<double, double> doZoom(int x, int y);
    };
} /* namespace neneofprogramming */

#endif /* ZOOMLIST_H_ */