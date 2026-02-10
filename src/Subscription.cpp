#include <ReactiveLitepp/Subscription.h>

namespace ReactiveLitepp
{
    Subscription::Subscription(std::function<void()> unsub, std::function<bool()> valid)
        : unsubscribeFunc(std::move(unsub)), isValidFunc(std::move(valid)) {
    }

    void Subscription::Unsubscribe() {
        if (unsubscribeFunc) {
            unsubscribeFunc();
            unsubscribeFunc = nullptr;
            isValidFunc = nullptr;
        }
    }

    bool Subscription::IsValid() const {
        return isValidFunc && isValidFunc();
    }
}
