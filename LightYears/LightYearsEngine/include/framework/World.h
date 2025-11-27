#pragma once
#include <SFML/Graphics.hpp>

#include "framework/Core.h"

namespace LightYears
{
    class Actor;
    class Application;
    class World
    {
    public:
        World(Application *owningApp);

        void BeginPlayInternal();
        void TickInternal(float deletTime);
        void Render(sf::RenderWindow &window);

        virtual ~World();

        template <typename ActorType>
        weak<ActorType> SpawnActor();

    private:
        void BeginPlay();
        void Tick(float deletTime);

    private:
        Application *owningApp_{};
        bool beganPlay_{};

        List<shared<Actor>> actors_{};
        List<shared<Actor>> pendingActors_{};
    };

    template <typename ActorType>
    weak<ActorType> World::SpawnActor()
    {
        shared<ActorType> newActor{new ActorType{this}};
        pendingActors_.push_back(newActor);
        return newActor;
    }
}