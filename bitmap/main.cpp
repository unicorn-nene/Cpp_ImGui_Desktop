#include <iostream>

#include "FractalCreator.h"
#include "RGB.h"
#include "Zoom.h"

using namespace neneofprogramming;

int main()
{
    std::string bmpName = "test6.bmp";

    FractalCreator fractalCreator(800, 600);

    // fractalCreator.addRange(0.0, RGB(0, 0, 0));       // 0%
    // fractalCreator.addRange(0.3, RGB(250, 0, 0));     // 30%
    // fractalCreator.addRange(0.5, RGB(255, 255, 0));   // 50%
    // fractalCreator.addRange(1.0, RGB(255, 255, 255)); // 100%

    fractalCreator.addRange(0.0, RGB(230, 50, 100));  // 0%
    fractalCreator.addRange(0.3, RGB(255, 255, 200)); // 30%
    fractalCreator.addRange(0.5, RGB(230, 50, 100));  // 50%
    fractalCreator.addRange(1.0, RGB(255, 255, 200)); // 100%

    // fractalCreator.addRange(0.0, RGB(20, 10, 40));    // 深紫
    // fractalCreator.addRange(0.2, RGB(255, 170, 220)); // 外层淡粉
    // fractalCreator.addRange(0.4, RGB(255, 120, 180)); // 粉红
    // fractalCreator.addRange(0.6, RGB(230, 50, 100));  // 玫瑰红
    // fractalCreator.addRange(0.8, RGB(255, 200, 50));  // 金黄
    // fractalCreator.addRange(1.0, RGB(255, 255, 200)); // 花心亮黄白

    fractalCreator.addZoom(Zoom(295, 202, 0.1));
    fractalCreator.addZoom(Zoom(312, 304, 0.1));
    fractalCreator.run(bmpName);

    std::cout << "Finished " << bmpName << std::endl;

    return 0;
}