#include "gameFramework/GameApplication.h"
#include "framework/World.h"
#include "framework/Actor.h"
#include "config.h"

LightYears::Application *GetApplication()
{
    return new LightYears::GameApplication();
}

std::unique_ptr<LightYears::Application> GetApplicationSmart()
{
    return std::make_unique<LightYears::GameApplication>();
}

namespace LightYears
{
    GameApplication::GameApplication()
        : Application{600, 980, "LightYears", sf::Style::Titlebar | sf::Style::Close}
    {
        weak<World> newWorld = LoadWorld<World>();
        newWorld.lock()->SpawnActor<Actor>(); // lock() 会尝试把 weak_ptr 转换为 shared_ptr，如果对象还活着，就得到一个有效的 shared_ptr；如果对象已经被销毁，则得到空的 shared_ptr

        actorToDestory_ = newWorld.lock()->SpawnActor<Actor>();
        actorToDestory_.lock()->SetTexture(GetResoureDir() + "SpaceShooterRedux/PNG/playerShip1_blue.png");
        counter_ = 0;
    }

    void GameApplication::Tick(float deltaTime)
    {
        counter_ += deltaTime;
        if (counter_ > 2.f)
        {
            if (!actorToDestory_.expired())
            {
                actorToDestory_.lock()->Destory();
            }
        }
    }
} // namespace LightYears
