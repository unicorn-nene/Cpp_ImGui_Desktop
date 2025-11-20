#ifndef RGB_H_
#define RGB_H_

namespace neneofprogramming
{

    /**
     * @brief RGB 类, 三原色
     *
     */
    struct RGB
    {
        double r_{};
        double g_{};
        double b_{};

        RGB(double r, double g, double b);
    };

    RGB operator-(const RGB &first, const RGB &second);
} /* namespace neneofprogramming */

#endif /* RGB */