#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <string>
#include <vector>
#include <memory>

using namespace ReactiveLitepp;

struct ComplexType {
    int id;
    std::string name;
    std::vector<int> values;
    
    ComplexType(int i = 0, std::string n = "") : id(i), name(std::move(n)) {}
    
    bool operator==(const ComplexType& other) const {
        return id == other.id && name == other.name && values == other.values;
    }
};

TEST_CASE("Property with complex types", "[property][advanced][complex]") {
    SECTION("Property with custom struct") {
        Property<ComplexType> obj = ComplexType(1, "Test");
        
        ComplexType retrieved = obj.Get();
        REQUIRE(retrieved.id == 1);
        REQUIRE(retrieved.name == "Test");
        
        ComplexType newObj(2, "Updated");
        obj.Set(newObj);
        
        retrieved = obj;
        REQUIRE(retrieved.id == 2);
        REQUIRE(retrieved.name == "Updated");
    }
    
    SECTION("Property with vector") {
        Property<std::vector<int>> numbers = std::vector<int>{1, 2, 3, 4, 5};
        
        std::vector<int> vec = numbers;
        REQUIRE(vec.size() == 5);
        REQUIRE(vec[0] == 1);
        REQUIRE(vec[4] == 5);
        
        numbers = std::vector<int>{10, 20, 30};
        vec = numbers.Get();
        REQUIRE(vec.size() == 3);
        REQUIRE(vec[0] == 10);
    }
    
    SECTION("Property with shared_ptr") {
        auto ptr = std::make_shared<int>(42);
        Property<std::shared_ptr<int>> prop = ptr;
        
        auto retrieved = prop.Get();
        REQUIRE(*retrieved == 42);
        REQUIRE(retrieved.use_count() >= 2);  // prop and retrieved both hold reference
        
        prop = std::make_shared<int>(100);
        retrieved = prop;
        REQUIRE(*retrieved == 100);
    }
}

TEST_CASE("Property chaining and dependencies", "[property][advanced][chaining]") {
    SECTION("Dependent properties") {
        int baseValue = 10;
        
        Property<int> base(
            [&]() { return baseValue; },
            [&](int& value) { baseValue = value; }
        );
        
        Property<int> derived(
            [&]() { return base.Get() * 2; },
            [&](int& value) { base.Set(value / 2); }
        );
        
        REQUIRE(base.Get() == 10);
        REQUIRE(derived.Get() == 20);
        
        base.Set(20);
        REQUIRE(derived.Get() == 40);
        
        derived.Set(100);
        REQUIRE(base.Get() == 50);
        REQUIRE(derived.Get() == 100);
    }
    
    SECTION("Calculated property") {
        Property<double> width = 10.0;
        Property<double> height = 5.0;
        
        double area = width * height;
        REQUIRE(area == 50.0);
        
        width = 20.0;
        area = width * height;
        REQUIRE(area == 100.0);
    }
}

TEST_CASE("Property with lazy initialization", "[property][advanced][lazy]") {
    SECTION("Lazy getter that initializes on first access") {
        bool initialized = false;
        int expensiveValue = 0;
        int computeCount = 0;
        
        Property<int> lazy(
            [&]() {
                if (!initialized) {
                    expensiveValue = 42;  // Simulate expensive computation
                    initialized = true;
                    computeCount++;
                }
                return expensiveValue;
            },
            [&](int& value) { expensiveValue = value; initialized = true; }
        );
        
        REQUIRE_FALSE(initialized);
        REQUIRE(computeCount == 0);
        
        int value = lazy.Get();
        REQUIRE(initialized);
        REQUIRE(value == 42);
        REQUIRE(computeCount == 1);
        
        // Second access should not reinitialize
        value = lazy.Get();
        REQUIRE(value == 42);
        REQUIRE(computeCount == 1);  // Still 1, not incremented
        
        // Third access
        value = lazy.Get();
        REQUIRE(computeCount == 1);  // Still 1
    }
}

TEST_CASE("Property with caching", "[property][advanced][cache]") {
    SECTION("Cached getter with invalidation") {
        int computeCount = 0;
        int baseValue = 10;
        bool cacheValid = false;
        int cachedValue = 0;
        
        Property<int> cached(
            [&]() {
                if (!cacheValid) {
                    computeCount++;
                    cachedValue = baseValue * baseValue;
                    cacheValid = true;
                }
                return cachedValue;
            },
            [&](int& value) {
                baseValue = value;
                cacheValid = false;  // Invalidate cache
            }
        );
        
        REQUIRE(computeCount == 0);
        
        int val = cached.Get();
        REQUIRE(computeCount == 1);
        REQUIRE(val == 100);
        
        // Second get uses cache
        val = cached.Get();
        REQUIRE(computeCount == 1);  // Not incremented
        
        // Setting invalidates cache
        cached.Set(5);
        val = cached.Get();
        REQUIRE(computeCount == 2);  // Recomputed
        REQUIRE(val == 25);
    }
}

TEST_CASE("Property with formatting", "[property][advanced][format]") {
    SECTION("String property with automatic formatting") {
        std::string storage;
        
        Property<std::string> formatted(
            [&]() { return storage; },
            [&](std::string& value) {
                // Auto-capitalize
                if (!value.empty()) {
                    value[0] = std::toupper(value[0]);
                }
                storage = value;
            }
        );
        
        formatted = "hello";
        REQUIRE(formatted.Get() == "Hello");
        
        formatted = "world";
        REQUIRE(formatted.Get() == "World");
    }
}

TEST_CASE("Property with bounds checking", "[property][advanced][bounds]") {
    SECTION("Integer property with min/max bounds") {
        int value = 50;
        const int MIN_VALUE = 0;
        const int MAX_VALUE = 100;
        
        Property<int> bounded(
            [&]() { return value; },
            [&](int& newValue) {
                if (newValue < MIN_VALUE) {
                    value = MIN_VALUE;
                } else if (newValue > MAX_VALUE) {
                    value = MAX_VALUE;
                } else {
                    value = newValue;
                }
            }
        );
        
        REQUIRE(bounded.Get() == 50);
        
        bounded = 75;
        REQUIRE(bounded == 75);
        
        bounded = 150;  // Exceeds max
        REQUIRE(bounded == 100);
        
        bounded = -20;  // Below min
        REQUIRE(bounded == 0);
        
        bounded = 42;
        REQUIRE(bounded == 42);
    }
}

TEST_CASE("Property with side effects", "[property][advanced][sideeffects]") {
    SECTION("Property setter with logging") {
        std::vector<std::string> log;
        int value = 0;
        
        Property<int> logged(
            [&]() { return value; },
            [&](int& newValue) {
                log.push_back("Setting value from " + std::to_string(value) + 
                             " to " + std::to_string(newValue));
                value = newValue;
            }
        );
        
        REQUIRE(log.empty());
        
        logged = 10;
        REQUIRE(log.size() == 1);
        REQUIRE(log[0] == "Setting value from 0 to 10");
        
        logged = 20;
        REQUIRE(log.size() == 2);
        REQUIRE(log[1] == "Setting value from 10 to 20");
    }
    
    SECTION("Property with notification to other objects") {
        int value1 = 0;
        int value2 = 0;
        
        Property<int> master(
            [&]() { return value1; },
            [&](int& newValue) {
                value1 = newValue;
                value2 = newValue;  // Sync value2
            }
        );
        
        master = 42;
        REQUIRE(value1 == 42);
        REQUIRE(value2 == 42);
        
        master = 100;
        REQUIRE(value1 == 100);
        REQUIRE(value2 == 100);
    }
}

TEST_CASE("Property comparison operations", "[property][advanced][comparison]") {
    SECTION("Comparing property values") {
        Property<int> x = 10;
        Property<int> y = 20;
        Property<int> z = 10;
        
        REQUIRE(x.Get() == z.Get());
        REQUIRE(x.Get() != y.Get());
        REQUIRE(x.Get() < y.Get());
        
        // Using implicit conversion
        REQUIRE(static_cast<int>(x) == static_cast<int>(z));
        REQUIRE(static_cast<int>(x) != static_cast<int>(y));
    }
}
