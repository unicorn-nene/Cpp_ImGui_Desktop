#pragma once
#include <memory>

namespace LightYears
{
    class Application;
} // namespace lightYears

extern LightYears::Application *GetApplication(); // extern 表示这个函数在其他地方实现
extern std::unique_ptr<LightYears::Application> GetApplicationSmart();