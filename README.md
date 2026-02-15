# ReactiveLitepp

[![release](https://img.shields.io/github/v/release/herve-er/ReactiveLitepp)](https://github.com/herve-er/ReactiveLitepp/releases)

> **Version 1.0.0** — A lightweight C++ library that brings reactive property change notifications to modern C++

ReactiveLitepp provides a type-safe implementation of reactive programming patterns inspired by .NET's `INotifyPropertyChanged` interface. Build responsive applications with automatic property change notifications, event-driven architectures, and observable objects.

---

## ✨ Features

- **🔔 Event System** — Publish/subscribe pattern with type-safe event handlers
- **📦 Reactive Properties** — Custom getter/setter logic with validation and transformation
- **👀 Observable Objects** — Automatic property change notifications (`PropertyChanging` / `PropertyChanged`)
- **🎯 Type-Safe** — Modern C++20 with full template support
- **🧵 Thread-Safe** — Built-in mutex protection for event handlers
- **🚀 Header-Only Core** — Easy integration with CMake
- **♻️ RAII Support** — Scoped subscriptions for automatic cleanup

---

## 📦 Installation

### Using CMake (Recommended)

```bash
git clone https://github.com/herve-er/ReactiveLitepp.git
cd ReactiveLitepp
mkdir build && cd build
cmake ..
cmake --build .
```

### Using vcpkg

```bash
vcpkg install nameof
```

Then include ReactiveLitepp in your `CMakeLists.txt`:

```cmake
find_package(nameof CONFIG REQUIRED)
add_subdirectory(ReactiveLitepp)
target_link_libraries(your_target PRIVATE ReactiveLitepp)
```

**Requirements:**
- C++20 compatible compiler
- CMake 3.14+
- [`nameof`](https://github.com/Neargye/nameof) library (auto-fetched via vcpkg)

---

## 🚀 Quick Start

```cpp
#include <ReactiveLitepp/Event.h>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>

using namespace ReactiveLitepp;

// Simple event
Event<std::string> messageEvent;
auto sub = messageEvent.Subscribe([](const std::string& msg) {
    std::cout << "Received: " << msg << "\n";
});
messageEvent.Notify("Hello, World!");

// Reactive property with validation
int value = 0;
Property<int> score(
    [&]() { return value; },
    [&](int& newValue) { 
        value = std::clamp(newValue, 0, 100); 
    }
);
score = 150;  // Automatically clamped to 100
```

---

## 📚 Core Components

### 1. **Event<Args...>**

Type-safe publish/subscribe event system with support for multiple subscribers.

```cpp
Event<std::string, int> dataEvent;

auto sub = dataEvent.Subscribe([](const std::string& name, int value) {
    std::cout << name << ": " << value << "\n";
});

dataEvent.Notify("Temperature", 23);
sub.Unsubscribe();  // Manual cleanup
```

**RAII-style scoped subscriptions:**

```cpp
{
    auto scopedSub = dataEvent.SubscribeScoped([](auto name, auto val) {
        // Handle event
    });
} // Automatically unsubscribes when scope ends
```

---

### 2. **Property\<T>**

Reactive property wrapper with custom getter/setter logic.

```cpp
// Backing field required
std::string nameValue = "Alice";

Property<std::string> name(
    [&]() { return nameValue; },           // Getter
    [&](std::string& value) {              // Setter
        nameValue = value; 
    }
);

// Natural usage
name = "Bob";                   // Uses operator=
std::string n = name;           // Implicit conversion
std::cout << name << "\n";      // Stream output
```

**With validation:**

```cpp
int ageValue = 0;
Property<int> age(
    [&]() { return ageValue; },
    [&](int& value) {
        if (value < 0 || value > 120) {
            std::cout << "Invalid age!\n";
            return;  // Reject invalid values
        }
        ageValue = value;
    }
);
```

---

### 3. **ObservableObject**

Base class for objects with property change notifications.

```cpp
class Person : public ObservableObject {
public:
    Property<std::string> Name = Property<std::string>(
        [this]() { return _name; },
        [this](std::string& value) {
            SetPropertyValueAndNotify<&Person::Name>(_name, value);
        }
    );

    Property<int> Age = Property<int>(
        [this]() { return _age; },
        [this](int& value) {
            SetPropertyValueAndNotify<&Person::Age>(_age, value);
        }
    );

private:
    std::string _name = "John";
    int _age = 30;
};

// Usage
Person person;
person.PropertyChanged.Subscribe([](ObservableObject&, PropertyChangedArgs args) {
    std::cout << "Property changed: " << args.PropertyName() << "\n";
});

person.Name = "Jane";  // Triggers PropertyChanging + PropertyChanged events
person.Age = 25;
```

**Key Features:**
- `PropertyChanging` — Fires **before** value changes
- `PropertyChanged` — Fires **after** value changes
- `SetPropertyValueAndNotify<Member>()` — Smart change detection (only fires if value actually changes)

---

## 💡 Usage Examples

### Event System

```cpp
Event<int> numberEvent;

auto subA = numberEvent.Subscribe([](int n) {
    std::cout << "Square: " << (n * n) << "\n";
});

auto subB = numberEvent.Subscribe([](int n) {
    std::cout << "Double: " << (n * 2) << "\n";
});

numberEvent.Notify(5);
// Output:
// Square: 25
// Double: 10
```

### Property with Transformation

```cpp
double celsius = 0.0;
Property<double> temperature(
    [&]() { return celsius; },
    [&](double& value) {
        std::cout << "Changing: " << celsius << "°C → " << value << "°C\n";
        celsius = value;
    }
);

temperature = 25.0;
temperature = 30.5;
```

### Observable Object (Real-World Example)

```cpp
class ShoppingCart : public ObservableObject {
public:
    Property<int> ItemCount = Property<int>(
        [this]() { return _itemCount; },
        [this](int& value) {
            SetPropertyValueAndNotify<&ShoppingCart::ItemCount>(_itemCount, value);
        }
    );

    Property<double> TotalPrice = Property<double>(
        [this]() { return _totalPrice; },
        [this](double& value) {
            SetPropertyValueAndNotify<&ShoppingCart::TotalPrice>(_totalPrice, value);
        }
    );

private:
    int _itemCount = 0;
    double _totalPrice = 0.0;
};

// Monitor changes
ShoppingCart cart;
cart.PropertyChanged.Subscribe([&](ObservableObject&, PropertyChangedArgs args) {
    if (args.PropertyName() == "TotalPrice") {
        std::cout << "New total: $" << cart.TotalPrice << "\n";
    }
});

cart.ItemCount = 5;
cart.TotalPrice = 99.99;
```

---

## 📖 API Reference

### **Event<Args...>**

| Method | Description |
|--------|-------------|
| `Subscribe(Handler)` | Subscribe to event, returns `Subscription` |
| `SubscribeScoped(Handler)` | Subscribe with RAII cleanup |
| `Notify(Args...)` | Trigger event with arguments |
| `operator+=` | Shorthand for `Subscribe()` |

### **Property\<T>**

| Method | Description |
|--------|-------------|
| `Get()` | Retrieve current value |
| `Set(value)` | Update value |
| `operator T()` | Implicit conversion to `T` |
| `operator=(value)` | Assignment operator |
| `operator==(value)` | Equality comparison |

### **ObservableObject**

| Member | Description |
|--------|-------------|
| `PropertyChanging` | Event fired before property changes |
| `PropertyChanged` | Event fired after property changes |
| `SetPropertyValueAndNotify<Member>()` | Update field with change detection |

---

## 🔧 Building Examples

```bash
cd ReactiveLitepp
mkdir build && cd build
cmake ..
cmake --build .
./examples/ReactiveLitepp_examples  # Run demo
```

See [`examples/main.cpp`](examples/main.cpp) for comprehensive usage patterns.

---

## 📄 License

**ReactiveLitepp** is free to use in:
- ✅ Personal projects
- ✅ Open-source software

**Commercial Use:**  
For closed-source or commercial applications, please contact the author to obtain a commercial license.

📧 **Contact for licensing:** [Open an issue](https://github.com/herve-er/ReactiveLitepp/issues) or reach out via GitHub.

---

## ��� Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 🙏 Acknowledgments

Built with modern C++20 and inspired by reactive programming patterns from .NET and other frameworks.

**Dependencies:**
- [nameof](https://github.com/Neargye/nameof) — Compile-time reflection for C++

---

<div align="center">

**[⭐ Star this repo](https://github.com/herve-er/ReactiveLitepp)** if you find it useful!

Made with ❤️ by [herve-er](https://github.com/herve-er)

</div>
