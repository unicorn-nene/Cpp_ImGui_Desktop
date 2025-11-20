#ifndef ZOOM_H_
#define ZOOM_H_

namespace neneofprogramming
{
    /**
     * @brief 表示一次缩放操作（Zoom），定义了缩放中心点和缩放比例
     *
     */
    struct Zoom
    {
        int x_{0};          // 缩放中心的 x 坐标
        int y_{0};          // 缩放中心的 y 坐标
        double scale_{1.0}; // 缩放比例

        Zoom() {}
        Zoom(int x, int y, double scale)
            : x_(x), y_(y), scale_(scale) {}
    };

} /* namespace neneofprogramming */

#endif /* ZOOM_H_ */