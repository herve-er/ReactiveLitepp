#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <string>
#include <sstream>

using namespace ReactiveLitepp;

TEST_CASE("Property with custom getter/setter", "[property][basic]") {
    SECTION("Integer property with backing field") {
        int ageValue = 25;
        Property<int> age(
            [&]() { return ageValue; },
            [&](int& value) { ageValue = value; }
        );
        
        REQUIRE(age.Get() == 25);
        REQUIRE(age == 25);  // Implicit conversion
    }
    
    SECTION("String property with backing field") {
        std::string nameValue = "John";
        Property<std::string> name(
            [&]() { return nameValue; },
            [&](std::string& value) { nameValue = value; }
        );
        
        REQUIRE(name.Get() == "John");
    }
    
    SECTION("Set method updates backing field") {
        int value = 0;
        Property<int> prop(
            [&]() { return value; },
            [&](int& v) { value = v; }
        );
        
        prop.Set(42);
        REQUIRE(prop.Get() == 42);
        REQUIRE(value == 42);
    }
    
    SECTION("Assignment operator updates backing field") {
        int value = 10;
        Property<int> prop(
            [&]() { return value; },
            [&](int& v) { value = v; }
        );
        
        prop = 20;
        REQUIRE(prop.Get() == 20);
        REQUIRE(value == 20);
    }
    
    SECTION("Using property in expressions") {
        int xValue = 10;
        int yValue = 20;
        
        Property<int> x([&]() { return xValue; }, [&](int& v) { xValue = v; });
        Property<int> y([&]() { return yValue; }, [&](int& v) { yValue = v; });
        
        int sum = x + y;
        REQUIRE(sum == 30);
        
        int product = x.Get() * y.Get();
        REQUIRE(product == 200);
    }
}

TEST_CASE("Property with validation logic", "[property][validation]") {
    SECTION("Custom setter with validation") {
        int scoreValue = 50;
        bool validationFailed = false;
        
        Property<int> score(
            [&]() { return scoreValue; },
            [&](int& value) {
                if (value < 0 || value > 100) {
                    validationFailed = true;
                    return;  // Don't update
                }
                validationFailed = false;
                scoreValue = value;
            }
        );
        
        score.Set(75);
        REQUIRE_FALSE(validationFailed);
        REQUIRE(scoreValue == 75);
        
        score.Set(150);  // Should be rejected
        REQUIRE(validationFailed);
        REQUIRE(scoreValue == 75);  // Unchanged
        
        score.Set(-10);  // Should be rejected
        REQUIRE(validationFailed);
        REQUIRE(scoreValue == 75);  // Unchanged
    }
    
    SECTION("Setter with clamping") {
        int value = 0;
        
        Property<int> clamped(
            [&]() { return value; },
            [&](int& newValue) {
                value = std::max(0, std::min(100, newValue));
            }
        );
        
        clamped = 50;
        REQUIRE(value == 50);
        
        clamped = 150;
        REQUIRE(value == 100);  // Clamped to max
        
        clamped = -20;
        REQUIRE(value == 0);  // Clamped to min
    }
}

TEST_CASE("Property with transformation logic", "[property][transformation]") {
    SECTION("Getter with transformation") {
        std::string internalValue = "hello";
        
        Property<std::string> transformed(
            [&]() { return internalValue + " world"; },
            [&](std::string& value) { internalValue = value; }
        );
        
        REQUIRE(transformed.Get() == "hello world");
        
        transformed.Set("goodbye");
        REQUIRE(transformed.Get() == "goodbye world");
    }
    
    SECTION("Setter with transformation") {
        std::string value = "";
        
        Property<std::string> uppercased(
            [&]() { return value; },
            [&](std::string& newValue) {
                value = newValue;
                for (char& c : value) {
                    c = std::toupper(c);
                }
            }
        );
        
        uppercased = "hello";
        REQUIRE(value == "HELLO");
    }
}

TEST_CASE("Property tracking operations", "[property][tracking]") {
    SECTION("Track getter calls") {
        int getCount = 0;
        int value = 42;
        
        Property<int> tracked(
            [&]() { getCount++; return value; },
            [&](int& v) { value = v; }
        );
        
        REQUIRE(getCount == 0);
        
        tracked.Get();
        REQUIRE(getCount == 1);
        
        int x = tracked;
        REQUIRE(getCount == 2);
    }
    
    SECTION("Track setter calls") {
        int setCount = 0;
        int value = 0;
        
        Property<int> tracked(
            [&]() { return value; },
            [&](int& v) { setCount++; value = v; }
        );
        
        REQUIRE(setCount == 0);
        
        tracked.Set(10);
        REQUIRE(setCount == 1);
        
        tracked = 20;
        REQUIRE(setCount == 2);
    }
}

TEST_CASE("Property stream output operator", "[property][output]") {
    SECTION("Integer property output") {
        int value = 42;
        Property<int> prop([&]() { return value; }, [&](int& v) { value = v; });
        
        std::ostringstream oss;
        oss << prop;
        REQUIRE(oss.str() == "42");
    }
    
    SECTION("String property output") {
        std::string value = "Test";
        Property<std::string> prop([&]() { return value; }, [&](std::string& v) { value = v; });
        
        std::ostringstream oss;
        oss << prop;
        REQUIRE(oss.str() == "Test");
    }
}

TEST_CASE("Property edge cases", "[property][edge]") {
    SECTION("Empty string") {
        std::string value = "";
        Property<std::string> prop([&]() { return value; }, [&](std::string& v) { value = v; });
        
        REQUIRE(prop.Get() == "");
        REQUIRE(prop.Get().empty());
    }
    
    SECTION("Zero values") {
        int intValue = 0;
        Property<int> intProp([&]() { return intValue; }, [&](int& v) { intValue = v; });
        REQUIRE(intProp.Get() == 0);
        
        double doubleValue = 0.0;
        Property<double> doubleProp([&]() { return doubleValue; }, [&](double& v) { doubleValue = v; });
        REQUIRE(doubleProp.Get() == 0.0);
    }
    
    SECTION("Negative values") {
        int value = -42;
        Property<int> prop([&]() { return value; }, [&](int& v) { value = v; });
        REQUIRE(prop.Get() == -42);
    }
    
    SECTION("Maximum values") {
        int value = std::numeric_limits<int>::max();
        Property<int> prop([&]() { return value; }, [&](int& v) { value = v; });
        REQUIRE(prop.Get() == std::numeric_limits<int>::max());
    }
}

TEST_CASE("Property with exceptions in setters", "[property][exceptions]") {
    SECTION("Setter that throws exception") {
        int value = 10;
        
        Property<int> prop(
            [&]() { return value; },
            [&](int& v) {
                if (v < 0) {
                    throw std::invalid_argument("Value must be non-negative");
                }
                value = v;
            }
        );
        
        REQUIRE(prop.Get() == 10);
        
        prop = 20;
        REQUIRE(prop == 20);
        
        REQUIRE_THROWS_AS(prop.Set(-5), std::invalid_argument);
        REQUIRE(prop.Get() == 20);  // Value unchanged after exception
    }
}

TEST_CASE("Property computed dependencies", "[property][computed]") {
    SECTION("Dependent property values") {
        int baseValue = 10;
        
        Property<int> base([&]() { return baseValue; },
                          [&](int& v) { baseValue = v; });
        
        Property<int> doubled([&]() { return base.Get() * 2; },
                             [&](int& v) { base.Set(v / 2); });
        
        REQUIRE(base.Get() == 10);
        REQUIRE(doubled.Get() == 20);
        
        base.Set(20);
        REQUIRE(doubled.Get() == 40);
        
        doubled.Set(100);
        REQUIRE(base.Get() == 50);
    }
}
