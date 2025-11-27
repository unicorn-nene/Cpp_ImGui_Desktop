#pragma once
#include <SFML/Graphics.hpp>

#include "framework/Object.h"
#include "framework/Core.h"

namespace LightYears
{
    class World;
    class Actor : public Object
    {
    public:
        Actor(World *owingWorld, const std::string &texturePath = "");
        virtual ~Actor();

        void BeginPlayInternal();
        void TickInternal(float deltaTime);

        virtual void BeginPlay();
        virtual void Tick(float deltaTime);

        void SetTexture(const std::string &texturePath);
        void Render(sf::RenderWindow &window);

    private:
        World *owningWorld_{};
        bool hasBeganPlay_{};

        sf::Sprite sprite_{};
        sf::Texture texture_{};
    };
} // namespace LightYears
