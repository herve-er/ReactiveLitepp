#include <ReactiveLitepp/ScopedSubscription.h>

namespace ReactiveLitepp
{
    ScopedSubscription::ScopedSubscription(Subscription sub)
        : subscription(std::move(sub)) {
    }

    ScopedSubscription::ScopedSubscription(ScopedSubscription&& other) noexcept
        : subscription(std::move(other.subscription)) {
    }

    ScopedSubscription& ScopedSubscription::operator=(ScopedSubscription&& other) noexcept {
        if (this != &other) {
            subscription.Unsubscribe();
            subscription = std::move(other.subscription);
        }
        return *this;
    }

    ScopedSubscription::~ScopedSubscription() {
        subscription.Unsubscribe();
    }
}
