#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Event.h>
#include <memory>
#include <vector>

using namespace ReactiveLitepp;

TEST_CASE("Event lifetime - dangling event pointer", "[event][lifetime]") {
    SECTION("Subscription outlives event") {
        Subscription sub;
        
        {
            Event<int> event;
            sub = event.Subscribe([](int) {});
            REQUIRE(sub.IsValid());
        } // event destroyed here
        
        REQUIRE_FALSE(sub.IsValid());
        REQUIRE_NOTHROW(sub.Unsubscribe()); // Should not crash
    }
    
    SECTION("Scoped subscription outlives event") {
        auto scopedSub = std::make_unique<ScopedSubscription>();
        
        {
            Event<int> event;
            *scopedSub = event.SubscribeScoped([](int) {});
        } // event destroyed here
        
        REQUIRE_NOTHROW(scopedSub.reset()); // Should not crash on destruction
    }
}

TEST_CASE("Event lifetime - event in shared_ptr", "[event][lifetime]") {
    SECTION("Multiple subscriptions with shared event") {
        auto event = std::make_shared<Event<int>>();
        std::vector<Subscription> subs;
        
        int callCount = 0;
        
        for (int i = 0; i < 10; ++i) {
            subs.push_back(event->Subscribe([&](int) { callCount++; }));
        }
        
        event->Notify(1);
        REQUIRE(callCount == 10);
        
        // Destroy event but keep subscriptions
        event.reset();
        
        // All subscriptions should be invalid
        for (const auto& sub : subs) {
            REQUIRE_FALSE(sub.IsValid());
        }
    }
}

TEST_CASE("Event lifetime - event in unique_ptr", "[event][lifetime]") {
    SECTION("Move semantics with unique_ptr") {
        auto event1 = std::make_unique<Event<int>>();
        
        int callCount = 0;
        auto sub = event1->Subscribe([&](int) { callCount++; });
        
        event1->Notify(1);
        REQUIRE(callCount == 1);
        
        // Move the event
        auto event2 = std::move(event1);
        REQUIRE(event1 == nullptr);
        
        // Subscription should still be valid and work with event2
        event2->Notify(1);
        REQUIRE(callCount == 2);
        
        // Destroy event2
        event2.reset();
        REQUIRE_FALSE(sub.IsValid());
    }
}

TEST_CASE("Event lifetime - subscription cleanup", "[event][lifetime]") {
    SECTION("Many subscriptions created and destroyed") {
        Event<int> event;
        
        for (int iteration = 0; iteration < 100; ++iteration) {
            std::vector<ScopedSubscription> subs;
            
            for (int i = 0; i < 100; ++i) {
                subs.push_back(event.SubscribeScoped([](int) {}));
            }
            
            int callCount = 0;
            subs.push_back(event.SubscribeScoped([&](int) { callCount++; }));
            
            event.Notify(1);
            REQUIRE(callCount == 1);
            
            // All subs destroyed here
        }
        
        // Event should still be functional
        int finalCallCount = 0;
        auto finalSub = event.Subscribe([&](int) { finalCallCount++; });
        event.Notify(1);
        REQUIRE(finalCallCount == 1);
    }
}

TEST_CASE("Event lifetime - circular references", "[event][lifetime]") {
    SECTION("Handler captures subscription") {
        Event<int> event;
        
        // This creates a potential circular reference
        std::shared_ptr<Subscription> subPtr = std::make_shared<Subscription>();
        
        int callCount = 0;
        *subPtr = event.Subscribe([subPtr, &callCount](int) {
            callCount++;
            // Handler has reference to its own subscription
        });
        
        event.Notify(1);
        REQUIRE(callCount == 1);
        
        subPtr->Unsubscribe();
        subPtr.reset();
        
        // Should not crash
        event.Notify(2);
        REQUIRE(callCount == 1);
    }
}

TEST_CASE("Event lifetime - weak_ptr in handler", "[event][lifetime]") {
    SECTION("Handler uses weak_ptr to shared resource") {
        Event<int> event;
        auto resource = std::make_shared<int>(0);
        std::weak_ptr<int> weakResource = resource;
        
        auto sub = event.Subscribe([weakResource](int value) {
            auto res = weakResource.lock();
            if (res) {
                *res += value;
            }
        });
        
        event.Notify(10);
        REQUIRE(*resource == 10);
        
        // Destroy resource
        resource.reset();
        
        // Handler should handle null weak_ptr gracefully
        REQUIRE_NOTHROW(event.Notify(20));
    }
}

TEST_CASE("Event lifetime - multiple events sharing handlers", "[event][lifetime]") {
    SECTION("Handler captures state shared between events") {
        Event<int> event1;
        Event<int> event2;
        
        auto sharedState = std::make_shared<int>(0);
        
        auto sub1 = event1.Subscribe([sharedState](int v) { *sharedState += v; });
        auto sub2 = event2.Subscribe([sharedState](int v) { *sharedState += v * 2; });
        
        event1.Notify(10);
        REQUIRE(*sharedState == 10);
        
        event2.Notify(10);
        REQUIRE(*sharedState == 30); // 10 + 20
        
        sub1.Unsubscribe();
        event1.Notify(10);
        REQUIRE(*sharedState == 30); // Unchanged
        
        event2.Notify(10);
        REQUIRE(*sharedState == 50); // 30 + 20
    }
}

TEST_CASE("Event lifetime - subscription in container", "[event][lifetime]") {
    SECTION("Vector of subscriptions with events destroyed in different order") {
        std::vector<Subscription> allSubs;
        
        {
            Event<int> event1;
            Event<std::string> event2;
            Event<double> event3;
            
            allSubs.push_back(event1.Subscribe([](int) {}));
            allSubs.push_back(event2.Subscribe([](const std::string&) {}));
            allSubs.push_back(event3.Subscribe([](double) {}));
            
            REQUIRE(allSubs[0].IsValid());
            REQUIRE(allSubs[1].IsValid());
            REQUIRE(allSubs[2].IsValid());
        } // All events destroyed
        
        REQUIRE_FALSE(allSubs[0].IsValid());
        REQUIRE_FALSE(allSubs[1].IsValid());
        REQUIRE_FALSE(allSubs[2].IsValid());
        
        // Should be safe to unsubscribe
        for (auto& sub : allSubs) {
            REQUIRE_NOTHROW(sub.Unsubscribe());
        }
    }
}

TEST_CASE("Event lifetime - self-unsubscribing handler", "[event][lifetime]") {
    SECTION("Handler unsubscribes itself during notification") {
        Event<int> event;
        
        Subscription sub;
        int callCount = 0;
        
        sub = event.Subscribe([&](int) {
            callCount++;
            sub.Unsubscribe(); // Unsubscribe during handling
        });
        
        event.Notify(1);
        REQUIRE(callCount == 1);
        
        event.Notify(2);
        REQUIRE(callCount == 1); // Not called again
    }
}

TEST_CASE("Event lifetime - destruction order", "[event][lifetime]") {
    SECTION("Event destroyed before all subscriptions") {
        std::vector<Subscription> subs;
        
        auto event = std::make_unique<Event<int>>();
        
        for (int i = 0; i < 100; ++i) {
            subs.push_back(event->Subscribe([](int) {}));
        }
        
        // Destroy event first
        event.reset();
        
        // All subscriptions should be cleanly invalidated
        for (auto& sub : subs) {
            REQUIRE_FALSE(sub.IsValid());
        }
        
        // Cleanup should not crash
        subs.clear();
        REQUIRE(true);
    }
    
    SECTION("Subscriptions destroyed before event") {
        Event<int> event;
        
        {
            std::vector<Subscription> subs;
            for (int i = 0; i < 100; ++i) {
                subs.push_back(event.Subscribe([](int) {}));
            }
            // subs destroyed here
        }
        
        // Event should still be functional
        int callCount = 0;
        auto sub = event.Subscribe([&](int) { callCount++; });
        event.Notify(1);
        REQUIRE(callCount == 1);
    }
}
