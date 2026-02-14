#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <string>
#include <stdexcept>

using namespace ReactiveLitepp;

TEST_CASE("Property edge cases and error conditions", "[property][edge]") {
    SECTION("Property with empty string") {
        Property<std::string> empty = std::string("");
        REQUIRE(empty.Get() == "");
        REQUIRE(empty.Get().empty());
        
        empty = "not empty";
        REQUIRE(empty.Get() == "not empty");
        
        empty = "";
        REQUIRE(empty.Get().empty());
    }
    
    SECTION("Property with zero values") {
        Property<int> zero = 0;
        REQUIRE(zero.Get() == 0);
        
        Property<double> zeroDouble = 0.0;
        REQUIRE(zeroDouble.Get() == 0.0);
        
        Property<bool> falseBool = false;
        REQUIRE_FALSE(falseBool.Get());
    }
    
    SECTION("Property with negative values") {
        Property<int> negative = -42;
        REQUIRE(negative.Get() == -42);
        
        negative = -100;
        REQUIRE(negative == -100);
        
        Property<double> negativeDouble = -3.14;
        REQUIRE(negativeDouble.Get() == -3.14);
    }
    
    SECTION("Property with maximum values") {
        Property<int> maxInt = std::numeric_limits<int>::max();
        REQUIRE(maxInt.Get() == std::numeric_limits<int>::max());
        
        Property<int> minInt = std::numeric_limits<int>::min();
        REQUIRE(minInt.Get() == std::numeric_limits<int>::min());
    }
}

TEST_CASE("Property self-assignment", "[property][edge][assignment]") {
    SECTION("Self-assignment with value") {
        Property<int> value = 42;
        int temp = value;
        value = temp;
        
        REQUIRE(value == 42);
    }
    
    SECTION("Multiple consecutive assignments of same value") {
        Property<std::string> text = std::string("same");
        
        text = "same";
        REQUIRE(text.Get() == "same");
        
        text = "same";
        REQUIRE(text.Get() == "same");
        
        text = "same";
        REQUIRE(text.Get() == "same");
    }
}

TEST_CASE("Property with exception handling in custom setters", "[property][edge][exceptions]") {
    SECTION("Setter that throws exception on invalid value") {
        int value = 10;
        
        Property<int> validated(
            [&]() { return value; },
            [&](int& newValue) {
                if (newValue < 0) {
                    throw std::invalid_argument("Value must be non-negative");
                }
                value = newValue;
            }
        );
        
        REQUIRE(validated.Get() == 10);
        
        validated = 20;
        REQUIRE(validated == 20);
        
        REQUIRE_THROWS_AS(validated.Set(-5), std::invalid_argument);
        REQUIRE(validated.Get() == 20);  // Value unchanged after exception
    }
    
    SECTION("Getter that throws exception") {
        Property<int> exceptional(
            []() -> int { throw std::runtime_error("Getter error"); },
            [](int&) {}
        );
        
        REQUIRE_THROWS_AS(exceptional.Get(), std::runtime_error);
    }
}

TEST_CASE("Property with const correctness", "[property][edge][const]") {
    SECTION("Const property can be read") {
        const Property<int> constProp = 42;
        
        REQUIRE(constProp.Get() == 42);
        
        int value = constProp;
        REQUIRE(value == 42);
    }
    
    SECTION("Using const reference to property") {
        Property<std::string> prop = std::string("test");
        const Property<std::string>& constRef = prop;
        
        REQUIRE(constRef.Get() == "test");
        
        std::string value = constRef;
        REQUIRE(value == "test");
    }
}

TEST_CASE("Property rapid updates", "[property][edge][performance]") {
    SECTION("Many rapid sequential updates") {
        Property<int> counter = 0;
        
        for (int i = 1; i <= 1000; ++i) {
            counter = i;
        }
        
        REQUIRE(counter.Get() == 1000);
    }
    
    SECTION("Alternating between two values") {
        Property<bool> toggle = false;
        
        for (int i = 0; i < 100; ++i) {
            toggle = !toggle.Get();
        }
        
        REQUIRE(toggle.Get() == false);  // Started false, toggled even number of times
    }
}

TEST_CASE("Property with move semantics", "[property][edge][move]") {
    SECTION("Moving string values") {
        Property<std::string> source = std::string("movable");
        
        std::string moved = std::move(source.Get());
        REQUIRE(moved == "movable");
        
        // Source property internal value may be moved-from
        // But we can still assign a new value
        source = "new value";
        REQUIRE(source.Get() == "new value");
    }
}

TEST_CASE("Property with special characters in strings", "[property][edge][strings]") {
    SECTION("String with newlines") {
        Property<std::string> multiline = std::string("line1\nline2\nline3");
        REQUIRE(multiline.Get() == "line1\nline2\nline3");
    }
    
    SECTION("String with special characters") {
        Property<std::string> special = std::string("!@#$%^&*(){}[]|\\:;\"'<>?/");
        REQUIRE(special.Get() == "!@#$%^&*(){}[]|\\:;\"'<>?/");
    }
    
    SECTION("String with unicode") {
        Property<std::string> unicode = std::string("Hello ?? ??");
        REQUIRE(unicode.Get() == "Hello ?? ??");
    }
    
    SECTION("Very long string") {
        std::string longStr(10000, 'x');
        Property<std::string> longProp = longStr;
        
        REQUIRE(longProp.Get().length() == 10000);
        REQUIRE(longProp.Get() == longStr);
    }
}

TEST_CASE("Property getter/setter called count", "[property][edge][tracking]") {
    SECTION("Track how many times getter is called") {
        int getCount = 0;
        int value = 42;
        
        Property<int> tracked(
            [&]() { getCount++; return value; },
            [&](int& newValue) { value = newValue; }
        );
        
        REQUIRE(getCount == 0);
        
        tracked.Get();
        REQUIRE(getCount == 1);
        
        int x = tracked;
        REQUIRE(getCount == 2);
        
        int y = tracked.Get();
        REQUIRE(getCount == 3);
    }
    
    SECTION("Track how many times setter is called") {
        int setCount = 0;
        int value = 0;
        
        Property<int> tracked(
            [&]() { return value; },
            [&](int& newValue) { setCount++; value = newValue; }
        );
        
        REQUIRE(setCount == 0);
        
        tracked.Set(10);
        REQUIRE(setCount == 1);
        
        tracked = 20;
        REQUIRE(setCount == 2);
        
        tracked.Set(30);
        REQUIRE(setCount == 3);
    }
}

TEST_CASE("Property with different constructor argument orders", "[property][edge][constructors]") {
    SECTION("Getter first, setter second") {
        int value = 100;
        Property<int> prop(
            [&]() { return value; },
            [&](int& v) { value = v; }
        );
        
        REQUIRE(prop.Get() == 100);
        prop.Set(200);
        REQUIRE(value == 200);
    }
}
