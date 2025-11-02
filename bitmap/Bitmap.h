#ifndef BITMAP_H_
#define BITMAP_H_

#include <string>
#include <cstdint>
#include <memory>

using namespace std;

namespace neneofprogramming
{
    /**
     * @brief
     *
     */
    class Bitmap
    {
    private:
        int width_{0};
        int height_{0};
        unique_ptr<uint8_t[]> pPixels_{nullptr};

    public:
        Bitmap();
        Bitmap(int width, int height);
        void setPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue);
        bool write(string filename);
        virtual ~Bitmap();
    };

} /* namespace neneofprogramming */

#endif /* BITMAP_H_ */