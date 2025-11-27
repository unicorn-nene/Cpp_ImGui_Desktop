#pragma once
namespace LightYears
{
    class Object
    {
    public:
        Object();
        virtual ~Object();

        void Destory();
        bool IsPendingDestory() const { return isPendingDestory_; }

    private:
        bool isPendingDestory_{};
    };
} // namespace LightYears
