#include "ZoomList.h"
#include <iostream>

namespace neneofprogramming
{
    ZoomList::ZoomList()
    {
    }

    /**
     * @brief Construct a new Zoom List:: Zoom List object
     *
     * @param width 图像宽度
     * @param height 图像高度
     */
    ZoomList::ZoomList(int width, int height)
        : width_(width), height_(height) {}

    /**
     * @brief 添加一次新的缩放操作
     *
     * @param zoom 要添加的缩放操作对象
     */
    void ZoomList::add(const Zoom &zoom)
    {
        zooms_.push_back(zoom);

        xCenter_ += (zoom.x_ - width_ / 2) * scale_;
        yCenter_ += -(zoom.y_ - height_ / 2) * scale_;

        scale_ *= zoom.scale_;

        std::cout << xCenter_ << ", " << yCenter_ << ", " << scale_ << std::endl;
    }

    /**
     * @brief 根据当前缩放状态, 将屏幕坐标(x, y) 映射到 分型坐标(xFractal, yFractal)
     *
     * @param x 屏幕坐标 X（像素）
     * @param y 屏幕坐标 Y（像素）
     * @return 返回映射后的分形坐标 (xFractal, yFractal)
     */
    std::pair<double, double> ZoomList::doZoom(int x, int y)
    {
        double xFractal = (x - width_ / 2) * scale_ + xCenter_;
        double yFractal = (y - height_ / 2) * scale_ + yCenter_;

        return std::pair<double, double>(xFractal, yFractal);
    }
} /* namespace neneofprogramming */