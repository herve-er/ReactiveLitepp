#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <nameof.hpp>
#include <ReactiveLitepp/Event.h>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>

using namespace ReactiveLitepp;

// ============================================================================
// SECTION 1: Events - Publish/Subscribe Pattern
// ============================================================================

void DemonstrateEvents() {
	std::cout << "\n" << std::string(80, '=') << "\n";
	std::cout << "SECTION 1: Events - Publish/Subscribe Pattern\n";
	std::cout << std::string(80, '=') << "\n\n";

	// 1.1 Simple Event with single parameter
	std::cout << "--- 1.1 Simple Event ---\n";
	Event<std::string> messageEvent;

	auto sub1 = messageEvent.Subscribe([](const std::string& msg) {
		std::cout << "  Subscriber 1 received: " << msg << "\n";
	});

	messageEvent.Notify("Hello, World!");

	// 1.2 Event with multiple parameters
	std::cout << "\n--- 1.2 Event with Multiple Parameters ---\n";
	Event<std::string, int, double> dataEvent;

	auto sub2 = dataEvent.Subscribe([](const std::string& name, int count, double value) {
		std::cout << "  Data: " << name << ", Count: " << count << ", Value: " << value << "\n";
	});

	dataEvent.Notify("Temperature", 5, 23.7);

	// 1.3 Multiple subscribers
	std::cout << "\n--- 1.3 Multiple Subscribers ---\n";
	Event<int> numberEvent;

	auto subA = numberEvent.Subscribe([](int n) {
		std::cout << "  Subscriber A: Square = " << (n * n) << "\n";
	});

	auto subB = numberEvent.Subscribe([](int n) {
		std::cout << "  Subscriber B: Double = " << (n * 2) << "\n";
	});

	numberEvent.Notify(5);

	// 1.4 Unsubscribe
	std::cout << "\n--- 1.4 Unsubscribe ---\n";
	std::cout << "  Before unsubscribe:\n";
	numberEvent.Notify(3);

	subA.Unsubscribe();
	std::cout << "  After unsubscribing A:\n";
	numberEvent.Notify(3);

	// 1.5 Scoped Subscription (RAII)
	std::cout << "\n--- 1.5 Scoped Subscription (RAII) ---\n";
	{
		auto scopedSub = numberEvent.SubscribeScoped([](int n) {
			std::cout << "  Scoped subscriber: Triple = " << (n * 3) << "\n";
		});

		std::cout << "  Inside scope:\n";
		numberEvent.Notify(4);
	} // scopedSub automatically unsubscribes here

	std::cout << "  Outside scope (scoped sub auto-unsubscribed):\n";
	numberEvent.Notify(4);
}

// ============================================================================
// SECTION 2: Properties - Reactive Value Wrappers
// ============================================================================

void DemonstrateProperties() {
	std::cout << "\n" << std::string(80, '=') << "\n";
	std::cout << "SECTION 2: Properties - Reactive Value Wrappers\n";
	std::cout << std::string(80, '=') << "\n\n";

	// 2.1 Property with auto storage (simplest usage)
	std::cout << "--- 2.1 Property with Auto Storage ---\n";
	Property<int> age = 25;
	Property<std::string> name = std::string("Alice");

	std::cout << "  Name: " << name << ", Age: " << age << "\n";

	// Using Set/Get methods
	age.Set(30);
	std::cout << "  After Set(30): Age = " << age.Get() << "\n";

	// Using assignment operator
	name = "Bob";
	std::cout << "  After assignment: Name = " << name << "\n";

	// Using in expressions (implicit conversion)
	int doubled = age * 2;
	std::cout << "  Age * 2 = " << doubled << "\n";

	// 2.2 Property with custom getter/setter
	std::cout << "\n--- 2.2 Property with Custom Getter/Setter ---\n";
	double celsius = 0.0;

	Property<double> temperature(
		[&]() { return celsius; },  // Getter
		[&](double& value) {        // Setter
			std::cout << "  Temperature changing: " << celsius << "C -> " << value << "C\n";
			celsius = value;
		}
	);

	temperature = 25.0;
	temperature = 30.5;
	std::cout << "  Current temperature: " << temperature << "C\n";

	// 2.3 Property with validation
	std::cout << "\n--- 2.3 Property with Validation ---\n";
	int score = 0;

	Property<int> validatedScore(
		[&]() { return score; },
		[&](int& value) {
			if (value < 0) {
				std::cout << "  [X] Validation failed: Score cannot be negative!\n";
				return;  // Don't update
			}
			if (value > 100) {
				std::cout << "  [!] Clamping score to 100\n";
				score = 100;
				return;
			}
			score = value;
		}
	);

	validatedScore = 75;
	std::cout << "  Score: " << validatedScore << "\n";
	validatedScore = -10;  // Will be rejected
	std::cout << "  Score after rejected update: " << validatedScore << "\n";
	validatedScore = 150;  // Will be clamped
	std::cout << "  Score after clamped update: " << validatedScore << "\n";

	// 2.4 Property with AutoGetter/AutoSetter (internal storage + custom logic)
	std::cout << "\n--- 2.4 Property with AutoGetter/AutoSetter ---\n";
	Property<int> percentage(
		[](int& internalValue) { return internalValue; },  // AutoGetter
		[](int& newValue, int& internalValue) {            // AutoSetter
			// Clamp between 0 and 100
			internalValue = std::max(0, std::min(100, newValue));
			std::cout << "  Percentage set to: " << internalValue << "%\n";
		}
	);

	percentage = 50;
	percentage = 150;  // Auto-clamped
	percentage = -20;  // Auto-clamped
}

// ============================================================================
// SECTION 3: ObservableObject - Property Change Notifications
// ============================================================================

class Person : public ObservableObject {
public:
	// Simple properties with auto storage
	Property<std::string> FirstName = std::string("John");
	Property<std::string> LastName = std::string("Doe");
	Property<int> Age = 30;

	// Property with custom logic and notifications
	Property<std::string> Email = Property<std::string>(
		[this]() { return _email; },
		[this](std::string& value) {
			if (value.find('@') == std::string::npos) {
				std::cout << "  [X] Invalid email format!\n";
				return;
			}
			NotifyPropertyChanging<&Person::Email>();
			_email = value;
			NotifyPropertyChanged<&Person::Email>();
		}
	);

	// Property with AutoSetter and notifications
	Property<double> Salary = Property<double>(
		[](double& value) { return value; },
		[this](double& newValue, double& internalValue) {
			if (newValue < 0) return;  // Reject negative
			NotifyPropertyChanging<&Person::Salary>();
			internalValue = newValue;
			NotifyPropertyChanged<&Person::Salary>();
		}
	);

	// Using SetPropertyValue (recommended for simple properties)
	void SetAge(int newAge) {
		if (SetPropertyValue<&Person::Age>(Age, newAge)) {
			std::cout << "  [OK] Age updated to " << Age << "\n";
		} else {
			std::cout << "  = Age unchanged (same value)\n";
		}
	}

	void SetFirstName(const std::string& name) {
		SetPropertyValue<&Person::FirstName>(FirstName, name);
	}

	std::string GetFullName() const {
		return FirstName.Get() + " " + LastName.Get();
	}

private:
	std::string _email = "john.doe@example.com";
};

void DemonstrateObservableObject() {
	std::cout << "\n" << std::string(80, '=') << "\n";
	std::cout << "SECTION 3: ObservableObject - Property Change Notifications\n";
	std::cout << std::string(80, '=') << "\n\n";

	Person person;

	// 3.1 Subscribe to property changes
	std::cout << "--- 3.1 Property Change Events ---\n";

	auto changingSub = person.PropertyChanging.Subscribe(
		[](ObservableObject& obj, PropertyChangingArgs args) {
			std::cout << "  [CHANGING] PropertyChanging: " << args.PropertyName() << " is about to change\n";
		}
	);

	auto changedSub = person.PropertyChanged.Subscribe(
		[&person](ObservableObject& obj, PropertyChangedArgs args) {
			auto& p = static_cast<Person&>(obj);
			std::cout << "  [CHANGED] PropertyChanged: " << args.PropertyName() << " changed\n";
			if (args.PropertyName() == "Email") {
				std::cout << "     New email: " << p.Email << "\n";
			}
		}
	);

	// 3.2 Trigger property changes
	std::cout << "\n--- 3.2 Changing Properties ---\n";
	person.Email = "jane.smith@example.com";
	person.Salary = 75000.0;

	// 3.3 SetPropertyValue only fires events when value actually changes
	std::cout << "\n--- 3.3 SetPropertyValue (Smart Change Detection) ---\n";
	person.SetAge(30);  // Same value - no events
	person.SetAge(31);  // Different value - events fire

	// 3.4 Multiple property changes
	std::cout << "\n--- 3.4 Multiple Property Changes ---\n";
	person.SetFirstName("Jane");
	person.LastName = "Smith";
	person.Salary = 80000.0;

	std::cout << "\n  Final state: " << person.GetFullName()
		<< ", Age: " << person.Age
		<< ", Email: " << person.Email
		<< ", Salary: $" << person.Salary << "\n";
}

// ============================================================================
// SECTION 4: Real-World Example - Shopping Cart
// ============================================================================

class ShoppingCart : public ObservableObject {
public:
	Property<int> ItemCount = 0;
	Property<double> TotalPrice = 0.0;
	Property<bool> HasDiscount = false;

	void AddItem(const std::string& name, double price, int quantity = 1) {
		std::cout << "  Adding: " << quantity << "x " << name << " ($" << price << " each)\n";

		SetPropertyValue<&ShoppingCart::ItemCount>(ItemCount, ItemCount.Get() + quantity);

		double itemTotal = price * quantity;
		double discount = HasDiscount ? itemTotal * 0.1 : 0.0;
		double finalPrice = itemTotal - discount;

		SetPropertyValue<&ShoppingCart::TotalPrice>(TotalPrice, TotalPrice.Get() + finalPrice);

		if (discount > 0) {
			std::cout << "    [DISCOUNT] Discount applied: -$" << discount << "\n";
		}
	}

	void ApplyDiscount(bool apply) {
		if (SetPropertyValue<&ShoppingCart::HasDiscount>(HasDiscount, apply)) {
			std::cout << "  " << (apply ? "[OK] Discount code applied!" : "[X] Discount removed") << "\n";
		}
	}

	void Clear() {
		SetPropertyValue<&ShoppingCart::ItemCount>(ItemCount, 0);
		SetPropertyValue<&ShoppingCart::TotalPrice>(TotalPrice, 0.0);
		SetPropertyValue<&ShoppingCart::HasDiscount>(HasDiscount, false);
		std::cout << "  [CLEAR] Cart cleared\n";
	}
};

void DemonstrateRealWorldExample() {
	std::cout << "\n" << std::string(80, '=') << "\n";
	std::cout << "SECTION 4: Real-World Example - Shopping Cart\n";
	std::cout << std::string(80, '=') << "\n\n";

	ShoppingCart cart;

	// Monitor cart changes
	cart.PropertyChanged.Subscribe([&cart](ObservableObject&, PropertyChangedArgs args) {
		std::cout << "  [UPDATE] Cart Update: ";
		if (args.PropertyName() == "ItemCount") {
			std::cout << "Items: " << cart.ItemCount;
		} else if (args.PropertyName() == "TotalPrice") {
			std::cout << std::fixed << std::setprecision(2) << "Total: $" << cart.TotalPrice;
		} else if (args.PropertyName() == "HasDiscount") {
			std::cout << "Discount: " << (cart.HasDiscount.Get() ? "Yes" : "No");
		}
		std::cout << "\n";
	});

	// Shopping scenario
	std::cout << "--- Shopping Scenario ---\n";
	cart.AddItem("Laptop", 999.99);
	cart.AddItem("Mouse", 29.99, 2);

	std::cout << "\n";
	cart.ApplyDiscount(true);
	cart.AddItem("Keyboard", 79.99);  // This will have discount applied

	std::cout << "\n--- Final Cart ---\n";
	std::cout << "  Items: " << cart.ItemCount << "\n";
	std::cout << "  Total: $" << std::fixed << std::setprecision(2) << cart.TotalPrice << "\n";
	std::cout << "  Discount Active: " << (cart.HasDiscount.Get() ? "Yes" : "No") << "\n";

	std::cout << "\n";
	cart.Clear();
}

// ============================================================================
// SECTION 5: Advanced Patterns
// ============================================================================

void DemonstrateAdvancedPatterns() {
	std::cout << "\n" << std::string(80, '=') << "\n";
	std::cout << "SECTION 5: Advanced Patterns\n";
	std::cout << std::string(80, '=') << "\n\n";

	// 5.1 Computed Properties
	std::cout << "--- 5.1 Computed Properties ---\n";
	Property<double> width = 10.0;
	Property<double> height = 5.0;

	// Area is computed from width and height
	double area = width * height;
	std::cout << "  Initial area: " << area << "\n";

	width = 20.0;
	area = width * height;  // Recompute
	std::cout << "  After width change: " << area << "\n";

	// 5.2 Property Dependency Chain
	std::cout << "\n--- 5.2 Property Dependency Chain ---\n";
	int baseValue = 10;

	Property<int> base(
		[&]() { return baseValue; },
		[&](int& v) { baseValue = v; std::cout << "  Base set to: " << v << "\n"; }
	);

	Property<int> derived(
		[&]() { return base.Get() * 2; },
		[&](int& v) { base.Set(v / 2); }
	);

	std::cout << "  Base: " << base << ", Derived: " << derived << "\n";
	base = 20;
	std::cout << "  After base=20: Base: " << base << ", Derived: " << derived << "\n";
	derived = 100;
	std::cout << "  After derived=100: Base: " << base << ", Derived: " << derived << "\n";

	// 5.3 Event Broadcasting
	std::cout << "\n--- 5.3 Event Broadcasting ---\n";
	Event<std::string> logger;

	// Multiple logging destinations
	auto consoleLogger = logger.Subscribe([](const std::string& msg) {
		std::cout << "  [CONSOLE] " << msg << "\n";
	});

	auto fileLogger = logger.Subscribe([](const std::string& msg) {
		std::cout << "  [FILE] (simulated) " << msg << "\n";
	});

	logger.Notify("Application started");
	logger.Notify("User logged in");
	logger.Notify("Data saved");
}

// ============================================================================
// MAIN - Run All Demonstrations
// ============================================================================

int main() {
	std::cout << "\n";
	std::cout << "+============================================================================+\n";
	std::cout << "|                  ReactiveLitepp Library - Complete Demo                   |\n";
	std::cout << "|                  Reactive Programming for Modern C++                       |\n";
	std::cout << "+============================================================================+\n";

	try {
		DemonstrateEvents();
		DemonstrateProperties();
		DemonstrateObservableObject();
		DemonstrateRealWorldExample();
		DemonstrateAdvancedPatterns();

		std::cout << "\n" << std::string(80, '=') << "\n";
		std::cout << "[OK] All demonstrations completed successfully!\n";
		std::cout << std::string(80, '=') << "\n\n";

	} catch (const std::exception& e) {
		std::cerr << "[ERROR] Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}