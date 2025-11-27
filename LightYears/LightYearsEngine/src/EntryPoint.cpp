#include "EntryPoint.h"
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

#include "framework/Application.h"

int main()
{
    // LightYears::Application *app = GetApplication();

    std::unique_ptr<LightYears::Application> app = GetApplicationSmart();
    app->run();

    std::cout << "Done" << std::endl;

    // delete app;
    return 0;
}
