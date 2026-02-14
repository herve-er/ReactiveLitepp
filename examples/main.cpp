#include <iostream>
#include <string>
#include <vector>
#include <nameof.hpp>
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
			NotifyPropertyChanging<&Person::Email>();
			_email = value;
			NotifyPropertyChanged<&Person::Email>();
		}
	);

	// 3. Property with custom get/set (internal storage)
	Property<std::string> Adress =
		Property<std::string>(
			[this](std::string& value) { return value; },
			[this](std::string& newValue, std::string& internalValue) {
				std::cout << "Adress changing from '" << internalValue << "' to '" << newValue << "'\n";
				NotifyPropertyChanging<&Person::Adress>();
				internalValue = newValue;
				NotifyPropertyChanged<&Person::Adress>();
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
			std::cout << "Balance updated: $" << _balance << " -> $" << value << std::endl;
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
	person.PropertyChanged += [](ObservableObject& obj, PropertyChangedArgs args) {
		std::cout << "The propery: " << args.PropertyName() << " changed" << std::endl;
		};
	person.PropertyChanging += [](ObservableObject& obj, PropertyChangingArgs args) {
		std::cout << "The propery: " << args.PropertyName() << " is changing" << std::endl;
		};

	std::cout << "Initial Name: " << person.Name << std::endl;
	std::cout << "Initial Age: " << person.Age << std::endl;

	// Using Set method
	person.Name.Set("Jane Smith");
	person.Age.Set(30);

	std::cout << "After Set - Name: " << person.Name << std::endl;
	std::cout << "After Set - Age: " << person.Age << std::endl;

	// Using assignment operator
	person.Name = "Bob Johnson";
	person.Age = 35;

	std::cout << "After Assignment - Name: " << person.Name << std::endl;
	std::cout << "After Assignment - Age: " << person.Age << std::endl;

	// Using implicit conversion
	int age = person.Age;

	std::cout << "Using Implicit Conversion - Name: " << person.Name << ", Age: " << age << "\n\n";

	// Test 2: Custom get/set with logging
	std::cout << "--- Test 2: Custom Get/Set with Logging ---\n";
	std::cout << "Initial Email: " << person.Email << std::endl;

	person.Email.Set("jane.smith@example.com");
	std::cout << "Current Email: " << person.Email << std::endl;

	person.Email = "bob.johnson@example.com";
	std::cout << "Current Email: " << person.Email << "\n\n";

	// Test 2b: Custom get/set with logging
	std::cout << "--- Test 2: Custom Get/Set with Logging ---\n";
	std::cout << "Initial Adress: " << person.Email << std::endl;

	

	person.Adress.Set("01 av de France, Paris");
	std::cout << "Current Adress: " << person.Adress << std::endl;

	person.Adress = "88 av de France, Paris";
	std::cout << "Current Adress: " << person.Adress << "\n\n";

	// Test 3: Custom get/set with validation
	std::cout << "--- Test 3: Custom Get/Set with Validation ---\n";
	Account account;

	std::cout << "Initial Balance: $" << account.Balance << std::endl;

	account.Balance = 100.50;
	account.Balance = 250.75;

	// This should trigger validation error
	account.Balance = -50.0;
	std::cout << "Final Balance: $" << account.Balance << "\n\n";

	// Test 4: Using properties in expressions
	std::cout << "--- Test 4: Using Properties in Expressions ---\n";
	Property<int> x = int(10);
	Property<int> y = int(20);

	int sum = x + y;
	int product = x * y;

	std::cout << "x = " << x << ", y = " << y << std::endl;
	std::cout << "Sum: " << sum << std::endl;
	std::cout << "Product: " << product << std::endl;

	x = sum;
	std::cout << "After x = sum: x = " << x << "\n\n";

	std::cout << "=== All Tests Complete ===\n";
	return 0;
}