#include <complex>
#include "Mandelbrot.h"

using namespace std;

namespace neneofprogramming
{
    Mandelbrot::Mandelbrot()
    {
    }

    Mandelbrot::~Mandelbrot()
    {
    }

    /**
     * @brief 计算给定点在 Mandelbrot 集和中的迭代次数
     *
     * 静态方法，无需创建类实例即可使用。
     * 对于复平面上的点 (x,y)，计算迭代公式直到发散或达到最大迭代次数。
     *
     * @param x 复平面点的实部坐标
     * @param y 复平面点的虚部坐标
     * @return int 发散前的迭代次数。如果达到 MAX_ITERATIONS 则返回 MAX_ITERATIONS，
     *             表示该点可能属于曼德博集。
     *
     * @note 迭代公式：z₀ = 0, zₙ₊₁ = zₙ² + c，其中 c = x + yi
     *       发散条件：|zₙ| > 2 （即距离原点超过2）
     *
     */
    int Mandelbrot::getIterations(double x, double y)
    {
        complex<double> z = 0;
        complex<double> c(x, y);

        int iterations = 0;
        while (iterations < MAX_ITETATIONS)
        {
            z = z * z + c; // Mandelbrot 迭代公式

            if (abs(z) > 2) // 如果复数的模超过 2，则认为点脱离集合
                break;

            iterations++;
        }

        return iterations;
    }
}