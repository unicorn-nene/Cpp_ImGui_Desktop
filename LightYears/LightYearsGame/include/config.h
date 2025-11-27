#pragma once
#include <string>

std::string GetResoureDir()
{
#ifdef NOBUG // release build
    return "assets/";
#else
    return "E:/c_plus_plus/cplusplus_lightYear/LightYearsGame/assets/";
#endif
}
