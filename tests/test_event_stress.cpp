#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <ReactiveLitepp/Event.h>
#include <vector>
#include <random>
#include <algorithm>

using namespace ReactiveLitepp;

TEST_CASE("Event stress test - many handlers", "[event][stress]") {
    Event<int> event;
    std::vector<Subscription> subs;
    
    const int numHandlers = 10000;
    int totalCalls = 0;
    
    for (int i = 0; i < numHandlers; ++i) {
        subs.push_back(event.Subscribe([&](int) { totalCalls++; }));
    }
    
    event.Notify(1);
    REQUIRE(totalCalls == numHandlers);
}

TEST_CASE("Event stress test - many events", "[event][stress]") {
    std::vector<Event<int>> events(1000);
    std::vector<Subscription> subs;
    
    int totalCalls = 0;
    
    for (auto& event : events) {
        for (int i = 0; i < 10; ++i) {
            subs.push_back(event.Subscribe([&](int) { totalCalls++; }));
        }
    }
    
    for (auto& event : events) {
        event.Notify(1);
    }
    
    REQUIRE(totalCalls == 10000);
}

TEST_CASE("Event stress test - rapid subscribe/unsubscribe", "[event][stress]") {
    Event<int> event;
    
    const int iterations = 10000;
    int callCount = 0;
    
    for (int i = 0; i < iterations; ++i) {
        auto sub = event.Subscribe([&](int) { callCount++; });
        event.Notify(1);
        REQUIRE(callCount == i + 1);
        sub.Unsubscribe();
    }
    
    // No handlers left
    event.Notify(1);
    REQUIRE(callCount == iterations);
}

TEST_CASE("Event stress test - random operations", "[event][stress]") {
    Event<int> event;
    std::vector<Subscription> subs;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 2); // 0=subscribe, 1=unsubscribe, 2=notify
    
    int totalNotifications = 0;
    int expectedCalls = 0;
    
    for (int i = 0; i < 10000; ++i) {
        int op = opDist(gen);
        
        if (op == 0) {
            // Subscribe
            subs.push_back(event.Subscribe([&](int) { totalNotifications++; }));
            expectedCalls = 0; // Reset since we added a handler
        }
        else if (op == 1 && !subs.empty()) {
            // Unsubscribe random
            std::uniform_int_distribution<> indexDist(0, subs.size() - 1);
            int index = indexDist(gen);
            subs[index].Unsubscribe();
            subs.erase(subs.begin() + index);
            expectedCalls = 0;
        }
        else if (op == 2) {
            // Notify
            int activeSubs = std::count_if(subs.begin(), subs.end(), 
                [](const Subscription& s) { return s.IsValid(); });
            expectedCalls += activeSubs;
            event.Notify(1);
        }
    }
    
    // Verify that notifications were received
    REQUIRE(totalNotifications > 0);
}

TEST_CASE("Event stress test - deeply nested events", "[event][stress]") {
    SECTION("Handler triggers another event") {
        Event<int> event1;
        Event<int> event2;
        Event<int> event3;
        
        int count1 = 0, count2 = 0, count3 = 0;
        
        auto sub3 = event3.Subscribe([&](int v) { count3 += v; });
        auto sub2 = event2.Subscribe([&](int v) { 
            count2 += v;
            event3.Notify(v * 2);
        });
        auto sub1 = event1.Subscribe([&](int v) { 
            count1 += v;
            event2.Notify(v * 2);
        });
        
        event1.Notify(1);
        
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 2);
        REQUIRE(count3 == 4);
    }
}

TEST_CASE("Event stress test - memory usage", "[event][stress]") {
    SECTION("Creating and destroying many subscriptions") {
        Event<int> event;
        
        for (int iteration = 0; iteration < 1000; ++iteration) {
            std::vector<Subscription> tempSubs;
            
            for (int i = 0; i < 100; ++i) {
                tempSubs.push_back(event.Subscribe([](int) {}));
            }
            
            // All subscriptions destroyed here
        }
        
        // Event should still work fine
        int callCount = 0;
        auto sub = event.Subscribe([&](int) { callCount++; });
        event.Notify(1);
        REQUIRE(callCount == 1);
    }
}

TEST_CASE("Event stress test - large payload", "[event][stress]") {
    struct LargePayload {
        std::vector<int> data;
        LargePayload() : data(1000, 42) {}
    };
    
    Event<LargePayload> event;
    
    int callCount = 0;
    auto sub = event.Subscribe([&](const LargePayload& payload) {
        callCount++;
        REQUIRE(payload.data.size() == 1000);
        REQUIRE(payload.data[0] == 42);
    });
    
    for (int i = 0; i < 100; ++i) {
        event.Notify(LargePayload{});
    }
    
    REQUIRE(callCount == 100);
}

TEST_CASE("Event stress test - subscription churn", "[event][stress]") {
    Event<int> event;
    std::vector<Subscription> activeSubs;
    
    int totalCalls = 0;
    
    // Continuously add and remove subscriptions
    for (int i = 0; i < 1000; ++i) {
        // Add 10 subscriptions
        for (int j = 0; j < 10; ++j) {
            activeSubs.push_back(event.Subscribe([&](int) { totalCalls++; }));
        }
        
        // Notify
        event.Notify(1);
        
        // Remove 5 subscriptions
        for (int j = 0; j < 5 && !activeSubs.empty(); ++j) {
            activeSubs.front().Unsubscribe();
            activeSubs.erase(activeSubs.begin());
        }
    }
    
    REQUIRE(totalCalls > 0);
}

TEST_CASE("Event performance benchmark", "[event][benchmark][!benchmark]") {
    Event<int> event;
    
    BENCHMARK("Subscribe") {
        return event.Subscribe([](int) {});
    };
    
    auto sub = event.Subscribe([](int) {});
    
    BENCHMARK("Notify (1 handler)") {
        event.Notify(42);
    };
    
    std::vector<Subscription> subs;
    for (int i = 0; i < 100; ++i) {
        subs.push_back(event.Subscribe([](int) {}));
    }
    
    BENCHMARK("Notify (100 handlers)") {
        event.Notify(42);
    };
    
    BENCHMARK("Unsubscribe") {
        auto tempSub = event.Subscribe([](int) {});
        return tempSub.Unsubscribe();
    };
}
