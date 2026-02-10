#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Event.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace ReactiveLitepp;

TEST_CASE("Event thread safety - concurrent subscriptions", "[event][multithreading]") {
    Event<int> event;
    std::atomic<int> totalCalls{0};
    
    const int numThreads = 10;
    const int subscriptionsPerThread = 100;
    
    std::vector<std::thread> threads;
    std::vector<Subscription> allSubs;
    std::mutex subsMutex;
    
    // Multiple threads subscribing concurrently
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < subscriptionsPerThread; ++i) {
                auto sub = event.Subscribe([&](int) {
                    totalCalls.fetch_add(1, std::memory_order_relaxed);
                });
                
                std::lock_guard<std::mutex> lock(subsMutex);
                allSubs.push_back(std::move(sub));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    event.Notify(1);
    REQUIRE(totalCalls.load() == numThreads * subscriptionsPerThread);
}

TEST_CASE("Event thread safety - concurrent notifications", "[event][multithreading]") {
    Event<int> event;
    std::atomic<int> totalCalls{0};
    
    const int numHandlers = 100;
    const int numNotifyThreads = 10;
    const int notificationsPerThread = 100;
    
    std::vector<Subscription> subs;
    for (int i = 0; i < numHandlers; ++i) {
        subs.push_back(event.Subscribe([&](int value) {
            totalCalls.fetch_add(value, std::memory_order_relaxed);
        }));
    }
    
    std::vector<std::thread> threads;
    for (int t = 0; t < numNotifyThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < notificationsPerThread; ++i) {
                event.Notify(1);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    int expected = numHandlers * numNotifyThreads * notificationsPerThread;
    REQUIRE(totalCalls.load() == expected);
}

TEST_CASE("Event thread safety - subscribe and notify simultaneously", "[event][multithreading]") {
    Event<int> event;
    std::atomic<int> subscriptionCount{0};
    std::atomic<int> notificationCount{0};
    std::atomic<bool> stopFlag{false};
    
    std::vector<Subscription> subs;
    std::mutex subsMutex;
    
    // Thread that continuously subscribes
    std::thread subscriberThread([&]() {
        for (int i = 0; i < 1000; ++i) {
            auto sub = event.Subscribe([&](int) {
                notificationCount.fetch_add(1, std::memory_order_relaxed);
            });
            
            std::lock_guard<std::mutex> lock(subsMutex);
            subs.push_back(std::move(sub));
            subscriptionCount.fetch_add(1, std::memory_order_relaxed);
            
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    // Thread that continuously notifies
    std::thread notifierThread([&]() {
        while (subscriptionCount.load() < 1000) {
            event.Notify(1);
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    });
    
    subscriberThread.join();
    notifierThread.join();
    
    REQUIRE(subscriptionCount.load() == 1000);
}

TEST_CASE("Event thread safety - concurrent unsubscribe", "[event][multithreading]") {
    Event<int> event;
    std::atomic<int> callCount{0};
    
    const int numSubscriptions = 1000;
    std::vector<Subscription> subs;
    
    for (int i = 0; i < numSubscriptions; ++i) {
        subs.push_back(event.Subscribe([&](int) {
            callCount.fetch_add(1, std::memory_order_relaxed);
        }));
    }
    
    // Verify all subscriptions work
    event.Notify(1);
    REQUIRE(callCount.load() == numSubscriptions);
    callCount = 0;
    
    // Unsubscribe from multiple threads
    std::vector<std::thread> threads;
    const int subsPerThread = numSubscriptions / 10;
    
    for (int t = 0; t < 10; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < subsPerThread; ++i) {
                int index = t * subsPerThread + i;
                if (index < subs.size()) {
                    subs[index].Unsubscribe();
                }
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    event.Notify(1);
    REQUIRE(callCount.load() == 0);
}

TEST_CASE("Event thread safety - race condition on subscription destruction", "[event][multithreading]") {
    std::atomic<int> callCount{0};
    
    auto event = std::make_shared<Event<int>>();
    std::weak_ptr<Event<int>> weakEvent = event;
    
    std::vector<Subscription> subs;
    for (int i = 0; i < 100; ++i) {
        subs.push_back(event->Subscribe([&](int) {
            callCount.fetch_add(1, std::memory_order_relaxed);
        }));
    }
    
    // Thread that notifies
    std::thread notifierThread([weakEvent]() {
        for (int i = 0; i < 100; ++i) {
            auto event = weakEvent.lock();
            if (event) {
                event->Notify(1);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // Thread that destroys the event partway through
    std::thread destroyerThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        event.reset(); // Destroy the event while notifications might be happening
    });
    
    notifierThread.join();
    destroyerThread.join();
    
    // Should not crash (the test itself is the assertion)
    REQUIRE(true);
}

TEST_CASE("Event thread safety - stress test", "[event][multithreading][stress]") {
    Event<int> event;
    std::atomic<int> subscribeCount{0};
    std::atomic<int> unsubscribeCount{0};
    std::atomic<int> notifyCount{0};
    std::atomic<int> handlerCallCount{0};
    
    std::vector<Subscription> subs;
    std::mutex subsMutex;
    
    const int duration_ms = 1000;
    std::atomic<bool> stopFlag{false};
    
    // Thread 1: Subscribe continuously
    std::thread subscriberThread([&]() {
        while (!stopFlag.load()) {
            auto sub = event.Subscribe([&](int) {
                handlerCallCount.fetch_add(1, std::memory_order_relaxed);
            });
            
            {
                std::lock_guard<std::mutex> lock(subsMutex);
                subs.push_back(std::move(sub));
            }
            subscribeCount.fetch_add(1, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // Thread 2: Unsubscribe continuously
    std::thread unsubscriberThread([&]() {
        while (!stopFlag.load()) {
            std::lock_guard<std::mutex> lock(subsMutex);
            if (!subs.empty() && subs.size() > 10) {
                subs.front().Unsubscribe();
                subs.erase(subs.begin());
                unsubscribeCount.fetch_add(1, std::memory_order_relaxed);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(150));
        }
    });
    
    // Thread 3 & 4: Notify continuously
    auto notifierFunc = [&]() {
        while (!stopFlag.load()) {
            event.Notify(1);
            notifyCount.fetch_add(1, std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
    };
    
    std::thread notifierThread1(notifierFunc);
    std::thread notifierThread2(notifierFunc);
    
    // Run for specified duration
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    stopFlag = true;
    
    subscriberThread.join();
    unsubscriberThread.join();
    notifierThread1.join();
    notifierThread2.join();
    
    // Verify we did substantial work
    REQUIRE(subscribeCount.load() > 0);
    REQUIRE(notifyCount.load() > 0);
    REQUIRE(handlerCallCount.load() > 0);
}
