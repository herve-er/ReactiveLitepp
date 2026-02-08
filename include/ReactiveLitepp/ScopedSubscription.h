#pragma once
#include "Subscription.h"

namespace ReactiveLitepp
{
    class ScopedSubscription {
    public:
        ScopedSubscription() = default;
        explicit ScopedSubscription(Subscription sub);

        ScopedSubscription(const ScopedSubscription&) = delete;
        ScopedSubscription& operator=(const ScopedSubscription&) = delete;

        ScopedSubscription(ScopedSubscription&& other) noexcept;
        ScopedSubscription& operator=(ScopedSubscription&& other) noexcept;

        ~ScopedSubscription();

    private:
        Subscription subscription;
    };
}
