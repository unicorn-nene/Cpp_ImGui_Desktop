#pragma once
#include <SFML/Graphics.hpp>
#include "framework/Core.h"
#include "framework/World.h"

namespace LightYears
{
    class World;
    class Application
    {
    public:
        Application(unsigned int windowWidth, unsigned int windowHeight, const std::string &title, sf::Uint32 style);

        void run();

        template <typename WorldType>
        weak<WorldType> LoadWorld();

    private:
        void TickInternal(float deltaTime);
        void RenderInternal();

        /**
         * @brief
         *
         * @param deltaTime deltaTime = 当前帧开始时间 - 上一帧开始时间
         *
         */
        virtual void Tick(float deltaTime);
        void Render();

    private:
        sf::RenderWindow window_{}; // 渲染窗口
        float targetFrameRate_{};   // 设置固定帧率为 60fps
        sf::Clock tickClock_{};     // 计时器

        shared<World> currentWorld_{};
    };

    template <typename WorldType>
    weak<WorldType> Application::LoadWorld()
    {
        shared<WorldType> newWorld{new WorldType{this}};
        currentWorld_ = newWorld;
        currentWorld_->BeginPlayInternal();
        return newWorld;
    }
}