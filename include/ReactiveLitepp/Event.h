#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
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
		Subscription operator+=(Handler handler) {
			return Subscribe(std::move(handler));
		}
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
			std::lock_guard<std::mutex> lock(mutex);
			int id = ++currentId;
			handlers[id] = std::move(h);
			return id;
		}

		void Remove(int id) {
			std::lock_guard<std::mutex> lock(mutex);
			handlers.erase(id);
		}

		void Notify(Args... args) {
			// Copy handlers under lock to avoid iterator invalidation and race conditions
			std::vector<Handler> handlersCopy;
			{
				std::lock_guard<std::mutex> lock(mutex);
				handlersCopy.reserve(handlers.size());
				for (auto& [id, handler] : handlers) {
					handlersCopy.push_back(handler);
				}
			}

			// Call handlers without holding the lock to avoid deadlocks
			for (auto& handler : handlersCopy) {
				handler(args...);
			}
		}

	private:
		std::unordered_map<int, Handler> handlers;
		int currentId = 0;
		mutable std::mutex mutex;
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