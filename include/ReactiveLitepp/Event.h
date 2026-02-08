#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include "Subscription.h"
#include "ScopedSubscription.h"

namespace ReactiveLitepp
{
    template<typename... Args>
    class Event
    {
    public:
        using Handler = std::function<void(Args...)>;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

    private:
        class Impl;

    public:
        Event();
        Subscription Subscribe(Handler handler);
        ScopedSubscription SubscribeScoped(Handler handler);
        void Notify(Args... args);

    private:
        std::shared_ptr<Impl> impl;
    };

    // ==========================================
    // Internal Implementation (shared)
    // ==========================================
    template<typename... Args>
    class Event<Args...>::Impl {
    public:
        int Add(Handler h) {
            int id = ++currentId;
            handlers[id] = std::move(h);
            return id;
        }

        void Remove(int id) {
            handlers.erase(id);
        }

        void Notify(Args... args) {
            for (auto& [id, handler] : handlers)
                handler(args...);
        }

    private:
        std::unordered_map<int, Handler> handlers;
        int currentId = 0;
    };

    // ==========================================
    // Event Member Functions
    // ==========================================
    template<typename... Args>
    Event<Args...>::Event()
        : impl(std::make_shared<Impl>())
    {
    }

    template<typename... Args>
    Subscription Event<Args...>::Subscribe(Handler handler) {
        auto weakImpl = std::weak_ptr<Impl>(impl);
        int id = impl->Add(std::move(handler));
        
        return Subscription(
            [weakImpl, id]() mutable {
                auto impl = weakImpl.lock();
                if (impl) {
                    impl->Remove(id);
                    weakImpl.reset();
                }
            },
            [weakImpl]() {
                return !weakImpl.expired();
            }
        );
    }

    template<typename... Args>
    ScopedSubscription Event<Args...>::SubscribeScoped(Handler handler) {
        return ScopedSubscription(Subscribe(std::move(handler)));
    }

    template<typename... Args>
    void Event<Args...>::Notify(Args... args) {
        impl->Notify(args...);
    }
}