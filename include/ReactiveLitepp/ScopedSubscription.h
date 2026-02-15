#pragma once
#include "Subscription.h"

namespace ReactiveLitepp
{
    class ScopedSubscription {
    public:
        ScopedSubscription() = default;
        
        explicit ScopedSubscription(Subscription sub)
            : subscription(std::move(sub)) {
        }

        ScopedSubscription(const ScopedSubscription&) = delete;
        ScopedSubscription& operator=(const ScopedSubscription&) = delete;

        ScopedSubscription(ScopedSubscription&& other) noexcept
            : subscription(std::move(other.subscription)) {
        }

        ScopedSubscription& operator=(ScopedSubscription&& other) noexcept {
            if (this != &other) {
                subscription.Unsubscribe();
                subscription = std::move(other.subscription);
            }
            return *this;
        }

        ~ScopedSubscription() {
            subscription.Unsubscribe();
        }

    private:
        Subscription subscription;
    };
}
