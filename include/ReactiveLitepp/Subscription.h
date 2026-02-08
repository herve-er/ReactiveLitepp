#pragma once
#include <functional>

namespace ReactiveLitepp
{
    class Subscription {
    public:
        Subscription() = default;

        void Unsubscribe();
        bool IsValid() const;

    private:
        template<typename... Args>
        friend class Event;

        Subscription(std::function<void()> unsub, std::function<bool()> valid);

        std::function<void()> unsubscribeFunc;
        std::function<bool()> isValidFunc;
    };
}
