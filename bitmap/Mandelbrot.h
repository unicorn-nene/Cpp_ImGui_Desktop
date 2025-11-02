#ifndef MANDELBROT_H_
#define MANDELBROT_H_

namespace neneofprogramming
{
    /**
     * @brief Mandelbrot 集合计算类
     *
     */
    class Mandelbrot
    {
    public:
        static const int MAX_ITETATIONS = 1000;

    public:
        Mandelbrot();
        virtual ~Mandelbrot();

        static int getIterations(double x, double y);
    };
} /* namespace neneofprogramming */

#endif
