#pragma once
#include "framework/Application.h"

namespace LightYears
{
    class GameApplication : public Application
    {
    public:
        GameApplication();

        virtual void Tick(float deltaTime) override;

    private:
        float counter_{};
        weak<Actor> actorToDestory_{};
    };
}