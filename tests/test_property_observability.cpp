#include <catch2/catch_test_macros.hpp>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>
#include <string>
#include <vector>

using namespace ReactiveLitepp;

class TestObservableClass : public ObservableObject {
public:
    Property<int> Age = 0;
    Property<std::string> Name = std::string("Default");
    
    Property<std::string> Email = Property<std::string>(
        [this]() { return _email; },
        [this](std::string& value) {
            NotifyPropertyChanging<&TestObservableClass::Email>();
            _email = value;
            NotifyPropertyChanged<&TestObservableClass::Email>();
        }
    );
    
    Property<double> Balance = Property<double>(
        [](double& internalValue) { return internalValue; },
        [this](double& newValue, double& internalValue) {
            NotifyPropertyChanging<&TestObservableClass::Balance>();
            if (newValue >= 0) {
                internalValue = newValue;
            }
            NotifyPropertyChanged<&TestObservableClass::Balance>();
        }
    );
    
private:
    std::string _email = "default@example.com";
};

TEST_CASE("Property with ObservableObject - Custom getter/setter", "[property][observable][custom]") {
    SECTION("PropertyChanged event fires on custom property change") {
        TestObservableClass obj;
        
        std::string changedPropertyName;
        int changeCount = 0;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedPropertyName = args.PropertyName();
            changeCount++;
        });
        
        obj.Email = "new@example.com";
        
        REQUIRE(changeCount == 1);
        REQUIRE(changedPropertyName == "Email");
        REQUIRE(obj.Email.Get() == "new@example.com");
    }
    
    SECTION("PropertyChanging event fires before change") {
        TestObservableClass obj;
        
        std::string changingPropertyName;
        int changingCount = 0;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs args) {
            changingPropertyName = args.PropertyName();
            changingCount++;
        });
        
        obj.Email = "changing@example.com";
        
        REQUIRE(changingCount == 1);
        REQUIRE(changingPropertyName == "Email");
    }
    
    SECTION("Both PropertyChanging and PropertyChanged fire in order") {
        TestObservableClass obj;
        
        std::vector<std::string> events;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs args) {
            events.push_back("Changing:" + args.PropertyName());
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            events.push_back("Changed:" + args.PropertyName());
        });
        
        obj.Email = "test@example.com";
        
        REQUIRE(events.size() == 2);
        REQUIRE(events[0] == "Changing:Email");
        REQUIRE(events[1] == "Changed:Email");
    }
    
    SECTION("Multiple property changes tracked separately") {
        TestObservableClass obj;
        
        std::vector<std::string> changedProperties;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedProperties.push_back(args.PropertyName());
        });
        
        obj.Email = "email1@example.com";
        obj.Email = "email2@example.com";
        
        REQUIRE(changedProperties.size() == 2);
        REQUIRE(changedProperties[0] == "Email");
        REQUIRE(changedProperties[1] == "Email");
    }
}

TEST_CASE("Property with ObservableObject - AutoGetter/AutoSetter", "[property][observable][auto-custom]") {
    SECTION("PropertyChanged fires with AutoSetter") {
        TestObservableClass obj;
        
        std::string changedPropertyName;
        int changeCount = 0;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changedPropertyName = args.PropertyName();
            changeCount++;
        });
        
        obj.Balance = 100.50;
        
        REQUIRE(changeCount == 1);
        REQUIRE(changedPropertyName == "Balance");
        REQUIRE(obj.Balance == 100.50);
    }
    
    SECTION("PropertyChanged fires even when validation fails") {
        TestObservableClass obj;
        
        int changeCount = 0;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        obj.Balance = 50.0;
        REQUIRE(changeCount == 1);
        REQUIRE(obj.Balance == 50.0);
        
        // Negative value should be rejected but events still fire
        obj.Balance = -10.0;
        REQUIRE(changeCount == 2);
        REQUIRE(obj.Balance == 50.0);  // Value unchanged due to validation
    }
    
    SECTION("Unsubscribe stops notifications") {
        TestObservableClass obj;
        
        int changeCount = 0;
        
        auto sub = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            changeCount++;
        });
        
        obj.Balance = 100.0;
        REQUIRE(changeCount == 1);
        
        sub.Unsubscribe();
        
        obj.Balance = 200.0;
        REQUIRE(changeCount == 1);  // No additional notification
    }
}

TEST_CASE("Multiple subscribers to property changes", "[property][observable][multiple]") {
    SECTION("Multiple subscribers all receive notifications") {
        TestObservableClass obj;
        
        int count1 = 0;
        int count2 = 0;
        int count3 = 0;
        
        auto sub1 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count1++;
        });
        
        auto sub2 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count2++;
        });
        
        auto sub3 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count3++;
        });
        
        obj.Email = "test@example.com";
        
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 1);
        REQUIRE(count3 == 1);
    }
    
    SECTION("Unsubscribing one doesn't affect others") {
        TestObservableClass obj;
        
        int count1 = 0;
        int count2 = 0;
        
        auto sub1 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count1++;
        });
        
        auto sub2 = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs) {
            count2++;
        });
        
        obj.Email = "first@example.com";
        REQUIRE(count1 == 1);
        REQUIRE(count2 == 1);
        
        sub1.Unsubscribe();
        
        obj.Email = "second@example.com";
        REQUIRE(count1 == 1);  // Not incremented
        REQUIRE(count2 == 2);  // Still receiving notifications
    }
}

TEST_CASE("Property change tracking", "[property][observable][tracking]") {
    SECTION("Track all property changes") {
        TestObservableClass obj;
        
        struct PropertyChange {
            std::string propertyName;
            bool isChanging;
        };
        
        std::vector<PropertyChange> changes;
        
        auto subChanging = obj.PropertyChanging.Subscribe([&](ObservableObject&, PropertyChangingArgs args) {
            changes.push_back({args.PropertyName(), true});
        });
        
        auto subChanged = obj.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
            changes.push_back({args.PropertyName(), false});
        });
        
        obj.Email = "test1@example.com";
        obj.Balance = 100.0;
        obj.Email = "test2@example.com";
        
        REQUIRE(changes.size() == 6);  // 3 properties * 2 events each
        
        REQUIRE(changes[0].propertyName == "Email");
        REQUIRE(changes[0].isChanging == true);
        REQUIRE(changes[1].propertyName == "Email");
        REQUIRE(changes[1].isChanging == false);
        
        REQUIRE(changes[2].propertyName == "Balance");
        REQUIRE(changes[2].isChanging == true);
        REQUIRE(changes[3].propertyName == "Balance");
        REQUIRE(changes[3].isChanging == false);
        
        REQUIRE(changes[4].propertyName == "Email");
        REQUIRE(changes[4].isChanging == true);
        REQUIRE(changes[5].propertyName == "Email");
        REQUIRE(changes[5].isChanging == false);
    }
}
