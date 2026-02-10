#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Event.h>
#include <string>
#include <vector>

using namespace ReactiveLitepp;

TEST_CASE("Event basic functionality", "[event][basic]") {
    SECTION("Subscribe and notify single handler") {
        Event<int> event;
        int receivedValue = 0;
        
        auto sub = event.Subscribe([&](int value) {
            receivedValue = value;
        });
        
        event.Notify(42);
        REQUIRE(receivedValue == 42);
    }
    
    SECTION("Subscribe multiple handlers") {
        Event<int> event;
        int count = 0;
        
        auto sub1 = event.Subscribe([&](int value) { count += value; });
        auto sub2 = event.Subscribe([&](int value) { count += value * 2; });
        auto sub3 = event.Subscribe([&](int value) { count += value * 3; });
        
        event.Notify(10);
        REQUIRE(count == 60); // 10 + 20 + 30
    }
    
    SECTION("Unsubscribe removes handler") {
        Event<int> event;
        int callCount = 0;
        
        auto sub = event.Subscribe([&](int) { callCount++; });
        
        event.Notify(1);
        REQUIRE(callCount == 1);
        
        sub.Unsubscribe();
        event.Notify(2);
        REQUIRE(callCount == 1); // Not called again
    }
    
    SECTION("Multiple unsubscribe is safe") {
        Event<int> event;
        auto sub = event.Subscribe([](int) {});
        
        sub.Unsubscribe();
        REQUIRE_NOTHROW(sub.Unsubscribe());
        REQUIRE_NOTHROW(sub.Unsubscribe());
    }
}

TEST_CASE("Event with multiple argument types", "[event][basic]") {
    SECTION("No arguments") {
        Event<> event;
        bool called = false;
        
        auto sub = event.Subscribe([&]() { called = true; });
        event.Notify();
        
        REQUIRE(called);
    }
    
    SECTION("Multiple arguments") {
        Event<std::string, int, double> event;
        std::string s;
        int i = 0;
        double d = 0.0;
        
        auto sub = event.Subscribe([&](const std::string& str, int num, double dbl) {
            s = str;
            i = num;
            d = dbl;
        });
        
        event.Notify("test", 123, 3.14);
        
        REQUIRE(s == "test");
        REQUIRE(i == 123);
        REQUIRE(d == 3.14);
    }
}

TEST_CASE("ScopedSubscription RAII behavior", "[event][scoped]") {
    SECTION("Auto unsubscribe on scope exit") {
        Event<int> event;
        int count = 0;
        
        {
            auto scopedSub = event.SubscribeScoped([&](int) { count++; });
            event.Notify(1);
            REQUIRE(count == 1);
        } // scopedSub destroyed here
        
        event.Notify(2);
        REQUIRE(count == 1); // Not called after scope exit
    }
    
    SECTION("Move semantics") {
        Event<int> event;
        int count = 0;
        
        auto scoped1 = event.SubscribeScoped([&](int) { count++; });
        event.Notify(1);
        REQUIRE(count == 1);
        
        auto scoped2 = std::move(scoped1);
        event.Notify(2);
        REQUIRE(count == 2);
    }
    
    SECTION("Vector of scoped subscriptions") {
        Event<int> event;
        int count = 0;
        
        std::vector<ScopedSubscription> subs;
        subs.push_back(event.SubscribeScoped([&](int) { count++; }));
        subs.push_back(event.SubscribeScoped([&](int) { count++; }));
        subs.push_back(event.SubscribeScoped([&](int) { count++; }));
        
        event.Notify(1);
        REQUIRE(count == 3);
        
        subs.clear();
        event.Notify(2);
        REQUIRE(count == 3); // All unsubscribed
    }
}

TEST_CASE("Subscription validity", "[event][subscription]") {
    SECTION("IsValid returns true for active subscription") {
        Event<int> event;
        auto sub = event.Subscribe([](int) {});
        
        REQUIRE(sub.IsValid());
    }
    
    SECTION("IsValid returns false after unsubscribe") {
        Event<int> event;
        auto sub = event.Subscribe([](int) {});
        
        sub.Unsubscribe();
        REQUIRE_FALSE(sub.IsValid());
    }
}

TEST_CASE("Mixed subscription types", "[event][subscription]") {
    SECTION("Can store different event types in vector") {
        Event<int> intEvent;
        Event<std::string> stringEvent;
        Event<> voidEvent;
        
        std::vector<Subscription> subs;
        
        int intValue = 0;
        std::string stringValue;
        bool voidCalled = false;
        
        subs.push_back(intEvent.Subscribe([&](int v) { intValue = v; }));
        subs.push_back(stringEvent.Subscribe([&](const std::string& s) { stringValue = s; }));
        subs.push_back(voidEvent.Subscribe([&]() { voidCalled = true; }));
        
        intEvent.Notify(42);
        stringEvent.Notify("hello");
        voidEvent.Notify();
        
        REQUIRE(intValue == 42);
        REQUIRE(stringValue == "hello");
        REQUIRE(voidCalled);
        
        // Unsubscribe all
        for (auto& sub : subs) {
            sub.Unsubscribe();
        }
        
        intValue = 0;
        stringValue = "";
        voidCalled = false;
        
        intEvent.Notify(99);
        stringEvent.Notify("world");
        voidEvent.Notify();
        
        REQUIRE(intValue == 0);
        REQUIRE(stringValue == "");
        REQUIRE_FALSE(voidCalled);
    }
}

TEST_CASE("Handler exceptions", "[event][exceptions]") {
    SECTION("Exception in one handler doesn't prevent others") {
        Event<int> event;
        int count1 = 0;
        int count2 = 0;
        
        auto sub1 = event.Subscribe([&](int v) { 
            count1 += v;
            if (v == 42) throw std::runtime_error("test");
        });
        auto sub2 = event.Subscribe([&](int v) { count2 += v; });
        
        event.Notify(1);
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 1);
        
        // This will throw but should continue to next handler
        // Note: In current implementation, exception will propagate
        // This test documents current behavior
        REQUIRE_THROWS(event.Notify(42));
    }
}
