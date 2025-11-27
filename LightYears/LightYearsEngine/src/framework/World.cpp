#include "framework/World.h"
#include "framework/Core.h"
#include "framework/Actor.h"

namespace LightYears
{
    World::World(Application *owningApp)
        : owningApp_{owningApp},
          beganPlay_{false},
          actors_{},
          pendingActors_{}
    {
    }

    void World::BeginPlayInternal()
    {
        if (!beganPlay_)
        {
            beganPlay_ = true;
            BeginPlay();
        }
    }
    void World::TickInternal(float deltaTime)
    {

        for (shared<Actor> actor : pendingActors_)
        {
            actors_.push_back(actor);
            actor->BeginPlayInternal();
        }
        pendingActors_.clear();

        for (auto iter = actors_.begin(); iter != actors_.end();)
        {
            if (iter->get()->IsPendingDestory())
            {
                iter = actors_.erase(iter); // Returns : An iterator pointing to the next element (or end()).
            }
            else
            {
                iter->get()->TickInternal(deltaTime);
                ++iter;
            }
        }

        Tick(deltaTime);
    }

    World::~World()
    {
    }

    void World::BeginPlay()
    {
        LOG("Began Play");
    }

    void World::Tick(float deletTime)
    {
        LOG("Tick at frame rate %f", 1.f / deletTime);
    }

    void World::Render(sf::RenderWindow &window)
    {
        for (auto &actor : actors_)
        {
            actor->Render(window);
        }
    }
}