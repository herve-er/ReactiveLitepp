# ReactiveLitepp

[![release](https://img.shields.io/github/v/release/herve-er/ReactiveLitepp)](https://github.com/herve-er/ReactiveLitepp/releases)

> **Version 1.0.0** — A lightweight C++ library that brings reactive property change notifications to modern C++

ReactiveLitepp provides an elegant, type-safe implementation of reactive programming patterns inspired by .NET's `INotifyPropertyChanged` interface. Build responsive applications with automatic property change notifications, event-driven architectures, and observable objects.

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

### Option 1: Manual Installation (No Package Manager Required)

Since **both ReactiveLitepp and nameof are header-only**, you can use them by simply adding include paths:

**Step 1: Get the dependencies**
```bash
# Clone nameof (header-only dependency)
git clone https://github.com/Neargye/nameof.git

# Clone ReactiveLitepp
git clone https://github.com/herve-er/ReactiveLitepp.git
```

**Step 2: Option A - Using CMake**
```cmake
cmake_minimum_required(VERSION 3.14)
project(YourProject)

# Add include directories
include_directories(
    path/to/nameof/include
    path/to/ReactiveLitepp/include
)

add_executable(your_app main.cpp)
target_compile_features(your_app PRIVATE cxx_std_20)
```

**Step 2: Option B - Direct compiler flags**
```bash
# g++ or clang++
g++ main.cpp -std=c++20 \
    -Ipath/to/nameof/include \
    -Ipath/to/ReactiveLitepp/include \
    -o your_app

# MSVC
cl /std:c++20 /EHsc /Ipath\to\nameof\include /Ipath\to\ReactiveLitepp\include main.cpp
```

**Step 3: Include in your code**
```cpp
#include <nameof.hpp>
#include <ReactiveLitepp/Event.h>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>
```

---

### Option 2: Using vcpkg (Recommended)

```bash
vcpkg install nameof
```

Then include ReactiveLitepp in your `CMakeLists.txt`:

```cmake
find_package(nameof CONFIG REQUIRED)
add_subdirectory(ReactiveLitepp)
target_link_libraries(your_target PRIVATE ReactiveLitepp)
```

---

### Option 3: Building with CMake (For Development)

```bash
git clone https://github.com/herve-er/ReactiveLitepp.git
cd ReactiveLitepp
mkdir build 
cd build
cmake ..
cmake --build .
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
# ReactiveLitepp

[![release](https://img.shields.io/github/v/release/herve-er/ReactiveLitepp)](https://github.com/herve-er/ReactiveLitepp/releases)

> **Version 1.0.0** — A lightweight C++ library that brings reactive property change notifications to modern C++

ReactiveLitepp provides an elegant, type-safe implementation of reactive programming patterns inspired by .NET's `INotifyPropertyChanged` interface. Build responsive applications with automatic property change notifications, event-driven architectures, and observable objects.

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

### Option 1: Manual Installation (No Package Manager Required)

Since **both ReactiveLitepp and nameof are header-only**, you can use them by simply adding include paths:

**Step 1: Get the dependencies**
```bash
# Clone nameof (header-only dependency)
git clone https://github.com/Neargye/nameof.git

# Clone ReactiveLitepp
git clone https://github.com/herve-er/ReactiveLitepp.git
```

**Step 2: Option A - Using CMake**
```cmake
cmake_minimum_required(VERSION 3.14)
project(YourProject)

# Add include directories
include_directories(
    path/to/nameof/include
    path/to/ReactiveLitepp/include
)

add_executable(your_app main.cpp)
target_compile_features(your_app PRIVATE cxx_std_20)
```

**Step 2: Option B - Direct compiler flags**
```bash
# g++ or clang++
g++ main.cpp -std=c++20 \
    -Ipath/to/nameof/include \
    -Ipath/to/ReactiveLitepp/include \
    -o your_app

# MSVC
cl /std:c++20 /EHsc /Ipath\to\nameof\include /Ipath\to\ReactiveLitepp\include main.cpp
```

**Step 3: Include in your code**
```cpp
#include <nameof.hpp>
#include <ReactiveLitepp/Event.h>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>
```

---

### Option 2: Using vcpkg (Recommended)

```bash
vcpkg install nameof
```

Then include ReactiveLitepp in your `CMakeLists.txt`:

```cmake
find_package(nameof CONFIG REQUIRED)
add_subdirectory(ReactiveLitepp)
target_link_libraries(your_target PRIVATE ReactiveLitepp)
```

---

### Option 3: Building with CMake (For Development)

```bash
git clone https://github.com/herve-er/ReactiveLitepp.git
cd ReactiveLitepp
mkdir build 
cd build
cmake ..
cmake --build .
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
