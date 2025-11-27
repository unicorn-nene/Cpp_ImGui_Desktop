#include "framework/Object.h"
#include "framework/Core.h"

namespace LightYears
{
    Object::Object()
        : isPendingDestory_{false}
    {
    }

    Object::~Object()
    {
        LOG("Object Destory");
    }

    void Object::Destory()
    {
        isPendingDestory_ = true;
    }
}