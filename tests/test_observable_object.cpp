#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/ObservableObject.h>
#include <string>
#include <vector>

using namespace ReactiveLitepp;

class TestObservable : public ObservableObject {
public:
    int Age = 0;
    std::string Name = "";
    double Balance = 0.0;
    bool IsActive = false;
    std::vector<int> Numbers;
    
    // Public wrappers to test the protected SetPropertyValue method
    bool SetAge(int value) {
        return SetPropertyValue<&TestObservable::Age>(Age, value);
    }
    
    bool SetName(std::string value) {
        return SetPropertyValue<&TestObservable::Name>(Name ,value);
    }
    
    bool SetBalance(double value) {
        return SetPropertyValue<&TestObservable::Balance>(Balance, value);
    }
    
    bool SetIsActive(bool value) {
        return SetPropertyValue<&TestObservable::IsActive>(IsActive, value);
    }
    
    bool SetNumbers(std::vector<int> value) {
        return SetPropertyValue<&TestObservable::Numbers>(Numbers, value);
    }
    
    // Expose NotifyPropertyChanged for testing
    using ObservableObject::NotifyPropertyChanged;
};

TEST_CASE("SetPropertyValue basic functionality", "[observable][setpropertyvalue][basic]") {
    SECTION("SetPropertyValue with integer - value changes") {
        TestObservable obj;
        obj.Age = 25;
        
        bool result = obj.SetAge(30);
        
        REQUIRE(result == true);
        REQUIRE(obj.Age == 30);
    }
    
    SECTION("SetPropertyValue with integer - same value") {
        TestObservable obj;
        obj.Age = 25;
        
        bool result = obj.SetAge(25);
        
        REQUIRE(result == false);
        REQUIRE(obj.Age == 25);
    }
    
    SECTION("SetPropertyValue with string - value changes") {
        TestObservable obj;
        obj.Name = "John";
        
        bool result = obj.SetName("Jane");
        
        REQUIRE(result == true);
        REQUIRE(obj.Name == "Jane");
    }
    
    SECTION("SetPropertyValue with string - same value") {
        TestObservable obj;
        obj.Name = "John";
        
        bool result = obj.SetName("John");
        
        REQUIRE(result == false);
        REQUIRE(obj.Name == "John");
    }
    
    SECTION("SetPropertyValue with double") {
        TestObservable obj;
        obj.Balance = 100.50;
        
        bool result1 = obj.SetBalance(200.75);
        REQUIRE(result1 == true);
        REQUIRE(obj.Balance == 200.75);
        
        bool result2 = obj.SetBalance(200.75);
        REQUIRE(result2 == false);
        REQUIRE(obj.Balance == 200.75);
    }
    
    SECTION("SetPropertyValue with boolean") {
        TestObservable obj;
        obj.IsActive = false;
        
        bool result1 = obj.SetIsActive(true);
        REQUIRE(result1 == true);
        REQUIRE(obj.IsActive == true);
        
        bool result2 = obj.SetIsActive(true);
        REQUIRE(result2 == false);
        REQUIRE(obj.IsActive == true);
    }
}

TEST_CASE("SetPropertyValue event notifications", "[observable][setpropertyvalue][events]") {
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
        
        obj.SetAge(20);
        
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
        
        bool result = obj.SetAge(10);
        
        REQUIRE(result == false);
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
            capturedOldValue = testObj.Name;
            order.push_back("Changing");
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject& o, PropertyChangedArgs) {
            auto& testObj = static_cast<TestObservable&>(o);
            capturedNewValue = testObj.Name;
            order.push_back("Changed");
        });
        
        obj.SetName("After");
        
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
        
        obj.SetAge(42);
        
        REQUIRE(changingProperty == "Age");
        REQUIRE(changedProperty == "Age");
    }
}

TEST_CASE("SetPropertyValue with multiple properties", "[observable][setpropertyvalue][multiple]") {
    SECTION("Multiple properties can be set independently") {
        TestObservable obj;
        
        std::vector<std::string> changedProperties;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedProperties.push_back(args.PropertyName());
        });
        
        obj.SetAge(25);
        obj.SetName("Alice");
        obj.SetBalance(500.0);
        obj.SetIsActive(true);
        
        REQUIRE(changedProperties.size() == 4);
        REQUIRE(changedProperties[0] == "Age");
        REQUIRE(changedProperties[1] == "Name");
        REQUIRE(changedProperties[2] == "Balance");
        REQUIRE(changedProperties[3] == "IsActive");
        
        REQUIRE(obj.Age == 25);
        REQUIRE(obj.Name == "Alice");
        REQUIRE(obj.Balance == 500.0);
        REQUIRE(obj.IsActive == true);
    }
    
    SECTION("Same property set multiple times") {
        TestObservable obj;
        
        std::vector<int> values;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject& o, PropertyChangedArgs) {
            auto& testObj = static_cast<TestObservable&>(o);
            values.push_back(testObj.Age);
        });
        
        obj.SetAge(10);
        obj.SetAge(20);
        obj.SetAge(30);
        
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
        
        bool r1 = obj.SetAge(20);  // Change
        bool r2 = obj.SetAge(20);  // No change
        bool r3 = obj.SetAge(30);  // Change
        bool r4 = obj.SetAge(30);  // No change
        
        REQUIRE(r1 == true);
        REQUIRE(r2 == false);
        REQUIRE(r3 == true);
        REQUIRE(r4 == false);
        REQUIRE(changeCount == 2);  // Only 2 changes
    }
}

TEST_CASE("SetPropertyValue with complex types", "[observable][setpropertyvalue][complex]") {
    SECTION("Vector property") {
        TestObservable obj;
        obj.Numbers = {1, 2, 3};
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        bool r1 = obj.SetNumbers(std::vector<int>{4, 5, 6});
        REQUIRE(r1 == true);
        REQUIRE(obj.Numbers == std::vector<int>{4, 5, 6});
        REQUIRE(changeCount == 1);
        
        // Same value
        bool r2 = obj.SetNumbers(std::vector<int>{4, 5, 6});
        REQUIRE(r2 == false);
        REQUIRE(changeCount == 1);
    }
}

TEST_CASE("SetPropertyValue return value semantics", "[observable][setpropertyvalue][return]") {
    SECTION("Returns true when value changes") {
        TestObservable obj;
        obj.Age = 0;
        
        bool result = obj.SetAge(100);
        REQUIRE(result == true);
    }
    
    SECTION("Returns false when value is same") {
        TestObservable obj;
        obj.Age = 100;
        
        bool result = obj.SetAge(100);
        REQUIRE(result == false);
    }
    
    SECTION("Consecutive calls with different values all return true") {
        TestObservable obj;
        
        REQUIRE(obj.SetAge(1) == true);
        REQUIRE(obj.SetAge(2) == true);
        REQUIRE(obj.SetAge(3) == true);
    }
    
    SECTION("Can be used in conditional logic") {
        TestObservable obj;
        obj.Age = 10;
        
        int updateCount = 0;
        
        if (obj.SetAge(20)) {
            updateCount++;
        }
        
        if (obj.SetAge(20)) {
            updateCount++;  // This won't execute
        }
        
        if (obj.SetAge(30)) {
            updateCount++;
        }
        
        REQUIRE(updateCount == 2);
    }
}

TEST_CASE("SetPropertyValue with zero and edge values", "[observable][setpropertyvalue][edge]") {
    SECTION("Setting to zero") {
        TestObservable obj;
        obj.Age = 10;
        
        bool result = obj.SetAge(0);
        
        REQUIRE(result == true);
        REQUIRE(obj.Age == 0);
    }
    
    SECTION("Setting from zero to zero") {
        TestObservable obj;
        obj.Age = 0;
        
        bool result = obj.SetAge(0);
        
        REQUIRE(result == false);
        REQUIRE(obj.Age == 0);
    }
    
    SECTION("Setting empty string") {
        TestObservable obj;
        obj.Name = "NotEmpty";
        
        bool result = obj.SetName("");
        
        REQUIRE(result == true);
        REQUIRE(obj.Name == "");
    }
    
    SECTION("Setting empty string to empty string") {
        TestObservable obj;
        obj.Name = "";
        
        bool result = obj.SetName("");
        
        REQUIRE(result == false);
        REQUIRE(obj.Name == "");
    }
    
    SECTION("Negative values") {
        TestObservable obj;
        obj.Age = 10;
        
        bool result = obj.SetAge(-5);
        
        REQUIRE(result == true);
        REQUIRE(obj.Age == -5);
    }
}

TEST_CASE("SetPropertyValue integration with manual notification", "[observable][setpropertyvalue][integration]") {
    SECTION("Can mix SetPropertyValue with manual NotifyPropertyChanged") {
        TestObservable obj;
        
        std::vector<std::string> notifications;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            notifications.push_back(args.PropertyName());
        });
        
        // Use SetPropertyValue
        obj.SetAge(10);
        
        // Manual update with notification
        obj.Name = "Manual";
        obj.NotifyPropertyChanged<&TestObservable::Name>();
        
        // Use SetPropertyValue again
        obj.SetBalance(100.0);
        
        REQUIRE(notifications.size() == 3);
        REQUIRE(notifications[0] == "Age");
        REQUIRE(notifications[1] == "Name");
        REQUIRE(notifications[2] == "Balance");
    }
}

TEST_CASE("SetPropertyValue with multiple subscribers", "[observable][setpropertyvalue][subscribers]") {
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
        
        obj.SetAge(42);
        
        REQUIRE(subscriber1Count == 1);
        REQUIRE(subscriber2Count == 1);
        REQUIRE(subscriber3Count == 1);
        
        // Same value - no notifications
        obj.SetAge(42);
        
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
        
        obj.SetAge(10);
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 1);
        
        sub1.Unsubscribe();
        
        obj.SetAge(20);
        REQUIRE(count1 == 1);  // Not incremented
        REQUIRE(count2 == 2);  // Still receiving
    }
}

TEST_CASE("SetPropertyValue performance characteristics", "[observable][setpropertyvalue][performance]") {
    SECTION("Rapid consecutive calls") {
        TestObservable obj;
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        for (int i = 1; i <= 100; ++i) {
            obj.SetAge(i);
        }
        
        REQUIRE(obj.Age == 100);
        REQUIRE(changeCount == 100);
    }
    
    SECTION("Alternating values") {
        TestObservable obj;
        
        int changeCount = 0;
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        for (int i = 0; i < 100; ++i) {
            obj.SetIsActive(!obj.IsActive);
        }
        
        REQUIRE(changeCount == 100);
        REQUIRE(obj.IsActive == false);  // Started false, toggled even number of times
    }
}
