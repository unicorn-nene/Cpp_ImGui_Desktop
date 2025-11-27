#include <iostream>
#include <cstdio>

#include "framework/Application.h"
#include "framework/Core.h"
#include "framework/World.h"

namespace LightYears
{
    Application::Application(unsigned int windowWidth, unsigned int windowHeight, const std::string &title, sf::Uint32 style)
        : window_{sf::VideoMode(windowWidth, windowHeight), title, style},
          targetFrameRate_{60.f},
          tickClock_{},
          currentWorld_{nullptr}
    {
    }

    void Application::run()
    {
        tickClock_.restart();
        float accumulatedTime = 0.f;                    // 记录上一帧到当前帧的累计时间
        float targetDeltaTime = 1.f / targetFrameRate_; // 1 / 60fps 表示60帧时每一帧消耗的时间

        while (window_.isOpen())
        {
            sf::Event windowEvent;

            while (window_.pollEvent(windowEvent))
            {
                if (windowEvent.type == sf::Event::EventType::Closed)
                {
                    window_.close();
                }
            }

            // 每当累积时间超过目标帧时间，就认为可以更新一帧游戏逻辑
            float frameDeltaTime = tickClock_.restart().asSeconds();
            accumulatedTime += frameDeltaTime;
            while (accumulatedTime > targetDeltaTime)
            {
                accumulatedTime -= targetDeltaTime;
                TickInternal(targetDeltaTime);
                RenderInternal();
            }
        }
    }

    void Application::TickInternal(float deltaTime)
    {
        Tick(deltaTime);
        if (currentWorld_)
        {
            currentWorld_->TickInternal(deltaTime);
        }
    }

    void Application::RenderInternal()
    {
        window_.clear();
        Render();
        window_.display();
    }

    void Application::Tick(float deltaTime)
    {
    }

    void Application::Render()
    {
        if (currentWorld_)
        {
            currentWorld_->Render(window_);
        }
    }

} // namespace lightYear
