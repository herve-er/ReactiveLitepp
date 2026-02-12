#include <iostream>
#include <string>
#include <vector>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>

using namespace ReactiveLitepp;

class Person : public ObservableObject
{
public:
	// 1. Property without get/set (internal storage)
	Property<std::string> Name = std::string("John Doe");
	Property<int> Age = 25;

	// 2. Property with custom get/set
	Property<std::string> Email = Property<std::string>(
		[this]() { return _email; },
		[this](std::string& value) {
			std::cout << "Email changing from '" << _email << "' to '" << value << "'\n";
			_email = value;
		}
	);

	// 3. Property with custom get/set (internal storage)
	Property<std::string> Adress =
		Property<std::string>(
			[this](std::string& value) { return value; },
			[this](std::string& newValue, std::string& internalValue) {
				std::cout << "Adress changing from '" << internalValue << "' to '" << newValue << "'\n";
				internalValue = newValue;
			}
		);


	Person() {}

private:
	std::string _email = "default@example.com";
};

class Account
{
public:
	// Property with custom validation logic
	Property<double> Balance;

	Account() : Balance(
		[this]() { return _balance; },
		[this](double& value) {
			if (value < 0) {
				std::cout << "Warning: Negative balance not allowed! Keeping current value.\n";
				return;
			}
			std::cout << "Balance updated: $" << _balance << " -> $" << value << "\n";
			_balance = value;
		}
	) {
		_balance = 0.0;
	}

private:
	double _balance;
};

int main() {
	std::cout << "=== Testing Property Class ===\n\n";

	// Test 1: Auto get/set (internal storage)
	std::cout << "--- Test 1: Auto Get/Set (Internal Storage) ---\n";
	Person person;

	std::cout << "Initial Name: " << person.Name.Get() << "\n";
	std::cout << "Initial Age: " << person.Age.Get() << "\n";

	// Using Set method
	person.Name.Set("Jane Smith");
	person.Age.Set(30);

	std::cout << "After Set - Name: " << person.Name.Get() << "\n";
	std::cout << "After Set - Age: " << person.Age.Get() << "\n";

	// Using assignment operator
	person.Name = "Bob Johnson";
	person.Age = 35;

	std::cout << "After Assignment - Name: " << person.Name.Get() << "\n";
	std::cout << "After Assignment - Age: " << person.Age.Get() << "\n";

	// Using implicit conversion
	std::string name = person.Name;
	int age = person.Age;

	std::cout << "Using Implicit Conversion - Name: " << name << ", Age: " << age << "\n\n";

	// Test 2: Custom get/set with logging
	std::cout << "--- Test 2: Custom Get/Set with Logging ---\n";
	std::cout << "Initial Email: " << person.Email.Get() << "\n";

	person.Email.Set("jane.smith@example.com");
	std::cout << "Current Email: " << person.Email.Get() << "\n";

	person.Email = "bob.johnson@example.com";
	std::cout << "Current Email: " << person.Email.Get() << "\n\n";

	// Test 2b: Custom get/set with logging
	std::cout << "--- Test 2: Custom Get/Set with Logging ---\n";
	std::cout << "Initial Email: " << person.Email.Get() << "\n";

	person.Adress.Set("01 av de France, Paris");
	std::cout << "Current Adress: " << person.Adress.Get() << "\n";

	person.Adress.Set("88 av de France, Paris");
	std::cout << "Current Adress: " << person.Adress.Get() << "\n\n";

	// Test 3: Custom get/set with validation
	std::cout << "--- Test 3: Custom Get/Set with Validation ---\n";
	Account account;

	std::cout << "Initial Balance: $" << account.Balance.Get() << "\n";

	account.Balance = 100.50;
	account.Balance = 250.75;

	// This should trigger validation error
	account.Balance = -50.0;
	std::cout << "Final Balance: $" << account.Balance.Get() << "\n\n";

	// Test 4: Using properties in expressions
	std::cout << "--- Test 4: Using Properties in Expressions ---\n";
	Property<int> x = int(10);
	Property<int> y = int(20);

	int sum = x + y;
	int product = x.Get() * y.Get();

	std::cout << "x = " << x.Get() << ", y = " << y.Get() << "\n";
	std::cout << "Sum: " << sum << "\n";
	std::cout << "Product: " << product << "\n";

	x = sum;
	std::cout << "After x = sum: x = " << x.Get() << "\n\n";

	std::cout << "=== All Tests Complete ===\n";
	return 0;
}