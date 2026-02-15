#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>
#include <string>
#include <vector>

using namespace ReactiveLitepp;

class TestObservable : public ObservableObject {
public:
    Property<int> Age = Property<int>(
        [this]() { return _age; },
        [this](int& value) {
            ObservableObject::SetPropertyValueAndNotify<&TestObservable::Age>(_age, value);
        }
    );
    
    Property<std::string> Name = Property<std::string>(
        [this]() { return _name; },
        [this](std::string& value) {
            ObservableObject::SetPropertyValueAndNotify<&TestObservable::Name>(_name, value);
        }
    );
    
    Property<double> Balance = Property<double>(
        [this]() { return _balance; },
        [this](double& value) {
            ObservableObject::SetPropertyValueAndNotify<&TestObservable::Balance>(_balance, value);
        }
    );
    
    Property<bool> IsActive = Property<bool>(
        [this]() { return _isActive; },
        [this](bool& value) {
            ObservableObject::SetPropertyValueAndNotify<&TestObservable::IsActive>(_isActive, value);
        }
    );
    
    Property<std::vector<int>> Numbers = Property<std::vector<int>>(
        [this]() { return _numbers; },
        [this](std::vector<int>& value) {
            ObservableObject::SetPropertyValueAndNotify<&TestObservable::Numbers>(_numbers, value);
        }
    );
    
    TestObservable() : _age(0), _name(""), _balance(0.0), _isActive(false) {}
    
    // Expose NotifyPropertyChanged for testing
    using ObservableObject::NotifyPropertyChanged;

private:
    int _age;
    std::string _name;
    double _balance;
    bool _isActive;
    std::vector<int> _numbers;
};

TEST_CASE("ObservableObject basic functionality", "[observable][basic]") {
    SECTION("Property values can be set and retrieved") {
        TestObservable obj;
        
        obj.Age = 30;
        REQUIRE(obj.Age.Get() == 30);
        
        obj.Name = "Jane";
        REQUIRE(obj.Name.Get() == "Jane");
        
        obj.Balance = 200.75;
        REQUIRE(obj.Balance.Get() == 200.75);
        
        obj.IsActive = true;
        REQUIRE(obj.IsActive.Get() == true);
    }
}

TEST_CASE("ObservableObject event notifications", "[observable][events]") {
    SECTION("PropertyChanging and PropertyChanged fire when value changes") {
        TestObservable obj;
        obj.Age = 10;
        
        std::vector<std::string> events;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs args) {
            events.push_back("Changing:" + args.PropertyName());
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            events.push_back("Changed:" + args.PropertyName());
        });
        
        obj.Age = 20;
        
        REQUIRE(events.size() == 2);
        REQUIRE(events[0] == "Changing:Age");
        REQUIRE(events[1] == "Changed:Age");
    }
    
    SECTION("Events do NOT fire when value is the same") {
        TestObservable obj;
        obj.Age = 10;
        
        int eventCount = 0;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs) {
            eventCount++;
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            eventCount++;
        });
        
        obj.Age = 10;
        
        REQUIRE(eventCount == 0);  // No events fired
    }
    
    SECTION("Events fire in correct order: Changing before Changed") {
        TestObservable obj;
        obj.Name = "Before";
        
        std::vector<std::string> order;
        std::string capturedOldValue;
        std::string capturedNewValue;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject& o, PropertyChangingArgs) {
            auto& testObj = static_cast<TestObservable&>(o);
            capturedOldValue = testObj.Name.Get();
            order.push_back("Changing");
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject& o, PropertyChangedArgs) {
            auto& testObj = static_cast<TestObservable&>(o);
            capturedNewValue = testObj.Name.Get();
            order.push_back("Changed");
        });
        
        obj.Name = "After";
        
        REQUIRE(order.size() == 2);
        REQUIRE(order[0] == "Changing");
        REQUIRE(order[1] == "Changed");
        REQUIRE(capturedOldValue == "Before");
        REQUIRE(capturedNewValue == "After");
    }
    
    SECTION("Correct property name in event args") {
        TestObservable obj;
        
        std::string changingProperty;
        std::string changedProperty;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs args) {
            changingProperty = args.PropertyName();
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedProperty = args.PropertyName();
        });
        
        obj.Age = 42;
        
        REQUIRE(changingProperty == "Age");
        REQUIRE(changedProperty == "Age");
    }
}

TEST_CASE("ObservableObject with multiple properties", "[observable][multiple]") {
    SECTION("Multiple properties can be set independently") {
        TestObservable obj;
        
        std::vector<std::string> changedProperties;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedProperties.push_back(args.PropertyName());
        });
        
        obj.Age = 25;
        obj.Name = "Alice";
        obj.Balance = 500.0;
        obj.IsActive = true;
        
        REQUIRE(changedProperties.size() == 4);
        REQUIRE(changedProperties[0] == "Age");
        REQUIRE(changedProperties[1] == "Name");
        REQUIRE(changedProperties[2] == "Balance");
        REQUIRE(changedProperties[3] == "IsActive");
        
        REQUIRE(obj.Age.Get() == 25);
        REQUIRE(obj.Name.Get() == "Alice");
        REQUIRE(obj.Balance.Get() == 500.0);
        REQUIRE(obj.IsActive.Get() == true);
    }
    
    SECTION("Same property set multiple times") {
        TestObservable obj;
        
        std::vector<int> values;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject& o, PropertyChangedArgs) {
            auto& testObj = static_cast<TestObservable&>(o);
            values.push_back(testObj.Age.Get());
        });
        
        obj.Age = 10;
        obj.Age = 20;
        obj.Age = 30;
        
        REQUIRE(values.size() == 3);
        REQUIRE(values[0] == 10);
        REQUIRE(values[1] == 20);
        REQUIRE(values[2] == 30);
    }
    
    SECTION("Mixed changes and no-changes") {
        TestObservable obj;
        obj.Age = 10;
        
        int changeCount = 0;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        obj.Age = 20;  // Change
        obj.Age = 20;  // No change
        obj.Age = 30;  // Change
        obj.Age = 30;  // No change
        
        REQUIRE(changeCount == 2);  // Only 2 changes
    }
}

TEST_CASE("ObservableObject with complex types", "[observable][complex]") {
    SECTION("Vector property") {
        TestObservable obj;
        obj.Numbers = {1, 2, 3};
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        obj.Numbers = std::vector<int>{4, 5, 6};
        REQUIRE(obj.Numbers.Get() == std::vector<int>{4, 5, 6});
        REQUIRE(changeCount == 1);
        
        // Same value
        obj.Numbers = std::vector<int>{4, 5, 6};
        REQUIRE(changeCount == 1);
    }
}

TEST_CASE("ObservableObject with zero and edge values", "[observable][edge]") {
    SECTION("Setting to zero") {
        TestObservable obj;
        obj.Age = 10;
        
        obj.Age = 0;
        
        REQUIRE(obj.Age.Get() == 0);
    }
    
    SECTION("Setting from zero to zero") {
        TestObservable obj;
        obj.Age = 0;
        
        obj.Age = 0;
        
        REQUIRE(obj.Age.Get() == 0);
    }
    
    SECTION("Setting empty string") {
        TestObservable obj;
        obj.Name = "NotEmpty";
        
        obj.Name = "";
        
        REQUIRE(obj.Name.Get() == "");
    }
    
    SECTION("Setting empty string to empty string") {
        TestObservable obj;
        obj.Name = "";
        
        obj.Name = "";
        
        REQUIRE(obj.Name.Get() == "");
    }
    
    SECTION("Negative values") {
        TestObservable obj;
        obj.Age = 10;
        
        obj.Age = -5;
        
        REQUIRE(obj.Age.Get() == -5);
    }
}

TEST_CASE("ObservableObject integration with manual notification", "[observable][integration]") {
    SECTION("Property setters use SetPropertyValueAndNotify automatically") {
        TestObservable obj;
        
        std::vector<std::string> notifications;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            notifications.push_back(args.PropertyName());
        });
        
        // All property setters use SetPropertyValueAndNotify
        obj.Age = 10;
        obj.Name = "Alice";
        obj.Balance = 100.0;
        
        REQUIRE(notifications.size() == 3);
        REQUIRE(notifications[0] == "Age");
        REQUIRE(notifications[1] == "Name");
        REQUIRE(notifications[2] == "Balance");
    }
}

TEST_CASE("ObservableObject with multiple subscribers", "[observable][subscribers]") {
    SECTION("All subscribers receive notifications") {
        TestObservable obj;
        
        int subscriber1Count = 0;
        int subscriber2Count = 0;
        int subscriber3Count = 0;
        
        auto sub1 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            subscriber1Count++;
        });
        
        auto sub2 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            subscriber2Count++;
        });
        
        auto sub3 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            subscriber3Count++;
        });
        
        obj.Age = 42;
        
        REQUIRE(subscriber1Count == 1);
        REQUIRE(subscriber2Count == 1);
        REQUIRE(subscriber3Count == 1);
        
        // Same value - no notifications
        obj.Age = 42;
        
        REQUIRE(subscriber1Count == 1);
        REQUIRE(subscriber2Count == 1);
        REQUIRE(subscriber3Count == 1);
    }
    
    SECTION("Unsubscribing one doesn't affect others") {
        TestObservable obj;
        
        int count1 = 0;
        int count2 = 0;
        
        auto sub1 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count1++;
        });
        
        auto sub2 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count2++;
        });
        
        obj.Age = 10;
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 1);
        
        sub1.Unsubscribe();
        
        obj.Age = 20;
        REQUIRE(count1 == 1);  // Not incremented
        REQUIRE(count2 == 2);  // Still receiving
    }
}

TEST_CASE("ObservableObject performance characteristics", "[observable][performance]") {
    SECTION("Rapid consecutive calls") {
        TestObservable obj;
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        for (int i = 1; i <= 100; ++i) {
            obj.Age = i;
        }
        
        REQUIRE(obj.Age.Get() == 100);
        REQUIRE(changeCount == 100);
    }
    
    SECTION("Alternating values") {
        TestObservable obj;
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        for (int i = 0; i < 100; ++i) {
            obj.IsActive = !obj.IsActive.Get();
        }
        
        REQUIRE(changeCount == 100);
        REQUIRE(obj.IsActive.Get() == false);  // Started false, toggled even number of times
    }
}

