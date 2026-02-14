#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <string>
#include <sstream>

using namespace ReactiveLitepp;

TEST_CASE("Property with internal storage (auto get/set)", "[property][basic][auto]") {
    SECTION("Integer property initialization") {
        Property<int> age = 25;
        REQUIRE(age.Get() == 25);
        REQUIRE(age == 25);  // Implicit conversion
    }
    
    SECTION("String property initialization") {
        Property<std::string> name = std::string("John Doe");
        REQUIRE(name.Get() == "John Doe");
        std::string nameStr = name;
        REQUIRE(nameStr == "John Doe");
    }
    
    SECTION("Set method updates value") {
        Property<int> count = 0;
        count.Set(42);
        REQUIRE(count.Get() == 42);
    }
    
    SECTION("Assignment operator updates value") {
        Property<int> value = 10;
        value = 20;
        REQUIRE(value.Get() == 20);
        REQUIRE(value == 20);
    }
    
    SECTION("Multiple assignments") {
        Property<std::string> text = std::string("initial");
        text = "first";
        REQUIRE(text.Get() == "first");
        text = "second";
        REQUIRE(text.Get() == "second");
        text.Set("third");
        REQUIRE(text.Get() == "third");
    }
    
    SECTION("Using property in expressions") {
        Property<int> x = 10;
        Property<int> y = 20;
        
        int sum = x + y;
        REQUIRE(sum == 30);
        
        int product = x.Get() * y.Get();
        REQUIRE(product == 200);
        
        // Assign result back
        x = sum;
        REQUIRE(x == 30);
    }
    
    SECTION("Boolean property") {
        Property<bool> isActive = true;
        REQUIRE(isActive == true);
        REQUIRE(isActive.Get());
        
        isActive = false;
        REQUIRE(isActive == false);
        REQUIRE_FALSE(isActive.Get());
    }
    
    SECTION("Double property with precision") {
        Property<double> price = 99.99;
        REQUIRE(price.Get() == 99.99);
        
        price = 149.50;
        REQUIRE(price == 149.50);
    }
}

TEST_CASE("Property with custom getter/setter", "[property][basic][custom]") {
    SECTION("Custom getter and setter with external storage") {
        int backingField = 100;
        
        Property<int> prop(
            [&]() { return backingField; },
            [&](int& value) { backingField = value; }
        );
        
        REQUIRE(prop.Get() == 100);
        
        prop.Set(200);
        REQUIRE(prop.Get() == 200);
        REQUIRE(backingField == 200);
        
        prop = 300;
        REQUIRE(prop == 300);
        REQUIRE(backingField == 300);
    }
    
    SECTION("Custom setter with validation") {
        int value = 50;
        bool validationCalled = false;
        
        Property<int> prop(
            [&]() { return value; },
            [&](int& newValue) {
                validationCalled = true;
                if (newValue >= 0 && newValue <= 100) {
                    value = newValue;
                }
            }
        );
        
        prop.Set(75);
        REQUIRE(validationCalled);
        REQUIRE(value == 75);
        
        validationCalled = false;
        prop.Set(150);  // Should be rejected
        REQUIRE(validationCalled);
        REQUIRE(value == 75);  // Unchanged
    }
    
    SECTION("Custom getter with transformation") {
        std::string internalValue = "hello";
        
        Property<std::string> prop(
            [&]() { return internalValue + " world"; },
            [&](std::string& value) { internalValue = value; }
        );
        
        REQUIRE(prop.Get() == "hello world");
        
        prop.Set("goodbye");
        REQUIRE(prop.Get() == "goodbye world");
    }
    
    SECTION("Tracking set operations") {
        int value = 0;
        int setCount = 0;
        
        Property<int> prop(
            [&]() { return value; },
            [&](int& newValue) {
                setCount++;
                value = newValue;
            }
        );
        
        REQUIRE(setCount == 0);
        
        prop.Set(10);
        REQUIRE(setCount == 1);
        
        prop = 20;
        REQUIRE(setCount == 2);
        
        prop = 30;
        REQUIRE(setCount == 3);
    }
}

TEST_CASE("Property with AutoGetter/AutoSetter", "[property][basic][auto-custom]") {
    SECTION("AutoGetter with internal value transformation") {
        Property<int> prop(
            [](int& internalValue) { return internalValue * 2; },
            [](int& newValue, int& internalValue) { internalValue = newValue; }
        );
        
        // Internal value starts at default (0)
        REQUIRE(prop.Get() == 0);  // 0 * 2
        
        prop.Set(5);
        REQUIRE(prop.Get() == 10);  // 5 * 2
        
        prop = 10;
        REQUIRE(prop == 20);  // 10 * 2
    }
    
    SECTION("AutoSetter with validation and internal storage") {
        bool validationFailed = false;
        
        Property<int> prop(
            [](int& internalValue) { return internalValue; },
            [&validationFailed](int& newValue, int& internalValue) {
                if (newValue < 0) {
                    validationFailed = true;
                    return;  // Don't update
                }
                validationFailed = false;
                internalValue = newValue;
            }
        );
        
        prop.Set(10);
        REQUIRE(prop.Get() == 10);
        REQUIRE_FALSE(validationFailed);
        
        prop.Set(-5);
        REQUIRE(prop.Get() == 10);  // Unchanged
        REQUIRE(validationFailed);
    }
}

TEST_CASE("Property stream output operator", "[property][basic][output]") {
    SECTION("Integer property output") {
        Property<int> value = 42;
        std::ostringstream oss;
        oss << value;
        REQUIRE(oss.str() == "42");
    }
    
    SECTION("String property output") {
        Property<std::string> name = std::string("Test");
        std::ostringstream oss;
        oss << name;
        REQUIRE(oss.str() == "Test");
    }
    
    SECTION("Boolean property output") {
        Property<bool> flag = true;
        std::ostringstream oss;
        oss << std::boolalpha << flag;
        REQUIRE(oss.str() == "true");
    }
}

TEST_CASE("Property type conversions", "[property][basic][conversion]") {
    SECTION("Implicit conversion to underlying type") {
        Property<int> prop = 42;
        
        int value = prop;  // Implicit conversion
        REQUIRE(value == 42);
        
        // Use in function expecting int
        auto square = [](int x) { return x * x; };
        REQUIRE(square(prop) == 1764);
    }
    
    SECTION("Using property in conditional statements") {
        Property<bool> condition = true;
        
        if (condition) {
            REQUIRE(true);
        } else {
            REQUIRE(false);
        }
        
        condition = false;
        REQUIRE_FALSE(condition);
    }
    
    SECTION("Using property in arithmetic") {
        Property<double> x = 10.5;
        Property<double> y = 2.5;
        
        double result = x + y;
        REQUIRE(result == 13.0);
        
        result = x * y;
        REQUIRE(result == 26.25);
    }
}
