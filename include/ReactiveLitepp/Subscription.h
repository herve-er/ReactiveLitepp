#pragma once
#include <functional>

namespace ReactiveLitepp
{
    class Subscription {
    public:
        Subscription() = default;

        void Unsubscribe() {
            if (unsubscribeFunc) {
                unsubscribeFunc();
                unsubscribeFunc = nullptr;
                isValidFunc = nullptr;
            }
        }

        bool IsValid() const {
            return isValidFunc && isValidFunc();
        }

    private:
        template<typename... Args>
        friend class Event;

        Subscription(std::function<void()> unsub, std::function<bool()> valid)
            : unsubscribeFunc(std::move(unsub)), isValidFunc(std::move(valid)) {
        }

        std::function<void()> unsubscribeFunc;
        std::function<bool()> isValidFunc;
    };
}
