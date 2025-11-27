#include "framework/Actor.h"
#include "framework/Core.h"

namespace LightYears
{
    Actor::Actor(World *owningWorld, const std::string &texturePath)
        : owningWorld_{owningWorld},
          hasBeganPlay_{false},
          sprite_{},
          texture_{}
    {
        SetTexture(texturePath);
    }

    Actor::~Actor()
    {
        LOG("Actor is destory");
    }

    void Actor::BeginPlayInternal()
    {
        if (!hasBeganPlay_)
        {
            hasBeganPlay_ = true;
            BeginPlay();
        }
    }

    void Actor::TickInternal(float deltaTime)
    {
        if (!IsPendingDestory())
        {
            Tick(deltaTime);
        }
    }

    void Actor::BeginPlay()
    {
        LOG("Actor begin play");
    }

    void Actor::Tick(float deltaTime)
    {
        LOG("Actor Ticking");
    }

    void Actor::SetTexture(const std::string &texturePath)
    {
        texture_.loadFromFile(texturePath);
        sprite_.setTexture(texture_);

        int textureWidth = texture_.getSize().x;
        int textureHeight = texture_.getSize().y;
        sprite_.setTextureRect(sf::IntRect{sf::Vector2i{}, sf::Vector2i{textureWidth, textureHeight}});
    }

    void Actor::Render(sf::RenderWindow &window)
    {
        if (IsPendingDestory())
            return;

        window.draw(sprite_);
    }
} // namespace LightYears
