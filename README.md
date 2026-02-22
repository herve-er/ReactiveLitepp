# ReactiveLitepp

[![release](https://img.shields.io/github/v/release/herve-er/ReactiveLitepp)](https://github.com/herve-er/ReactiveLitepp/releases)

A lightweight C++20 library for reactive property change notifications, inspired by .NET’s `INotifyPropertyChanged`.

ReactiveLitepp provides **type-safe events**, **reactive properties**, and **observable objects** to help you build responsive applications with minimal boilerplate.

---

## ✨ Why ReactiveLitepp?

ReactiveLitepp brings modern reactive patterns to C++ in a clean, header-only design.

* 🔔 **Event System** — Type-safe publish/subscribe with scoped (RAII) subscriptions
* 📦 **Reactive Properties** — Custom getter/setter logic with validation and transformation
* 👀 **Observable Objects** — Automatic `PropertyChanging` / `PropertyChanged` notifications
* 📚 **Observable Collections** — Track collection mutations with change events
* 🎯 **Modern C++20** — Fully templated and type-safe
* 🧵 **Thread-Safe** — Built-in mutex protection for event handlers
* 🚀 **Header-Only Core** — Drop-in integration with any build system

---

## 📦 Installation

ReactiveLitepp depends on the header-only [`nameof`](https://github.com/Neargye/nameof) library.

You can integrate it in several ways:

---

### 1️⃣ Standalone (Header-Only)

Clone both repositories and include their headers in your project.

```bash
# Clone nameof (dependency)
git clone https://github.com/Neargye/nameof.git

# Clone ReactiveLitepp
git clone https://github.com/herve-er/ReactiveLitepp.git
```

### CMake Example

```cmake
cmake_minimum_required(VERSION 3.14)
project(YourProject LANGUAGES CXX)

add_executable(your_app main.cpp)

target_compile_features(your_app PRIVATE cxx_std_20)

target_include_directories(your_app PRIVATE
    path/to/nameof/include
    path/to/ReactiveLitepp/include
)
```

### Direct Compiler Usage

```bash
# g++ / clang++
g++ main.cpp -std=c++20 \
    -Ipath/to/nameof/include \
    -Ipath/to/ReactiveLitepp/include \
    -o your_app

# MSVC
cl /std:c++20 /EHsc ^
   /Ipath\to\nameof\include ^
   /Ipath\to\ReactiveLitepp\include ^
   main.cpp
```

---

### 2️⃣ Using vcpkg (Maintainer Registry)

ReactiveLitepp is available through the maintainer vcpkg registry.

Add this to your `vcpkg-configuration.json`:

```json
{
  "registries": [
    {
      "kind": "artifact",
      "location": "https://github.com/microsoft/vcpkg-ce-catalog/archive/refs/heads/main.zip",
      "name": "microsoft"
    },
    {
      "kind": "git",
      "repository": "https://github.com/herve-er/vcpkg-registery",
      "baseline": "HEAD",
      "packages": ["reactivelitepp"]
    }
  ]
}
```

Install:

```bash
vcpkg install reactivelitepp
```

Use in CMake:

```cmake
find_package(reactivelitepp CONFIG REQUIRED)

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE reactivelitepp::reactivelitepp)
```

---

### 3️⃣ Building the Repository (Development)

```bash
git clone https://github.com/herve-er/ReactiveLitepp.git
cd ReactiveLitepp
cmake -B build -S .
cmake --build build
```

**Requirements**

* C++20 compatible compiler
* CMake 3.14+
* `nameof` (automatically pulled via vcpkg for dev builds)

---

## 🚀 Quick Start

```cpp
#include <ReactiveLitepp/Event.h>
#include <ReactiveLitepp/Property.h>
#include <ReactiveLitepp/ObservableObject.h>
#include <ReactiveLitepp/ObservableCollection.h>

using namespace ReactiveLitepp;
```

---

## 🔔 Events

```cpp
Event<std::string> messageEvent;

auto sub = messageEvent.Subscribe([](const std::string& msg) {
    std::cout << "Received: " << msg << "\n";
});

messageEvent.Notify("Hello, World!");
```

* Type-safe handlers
* Thread-safe notifications
* RAII subscription cleanup

---

## 📦 Properties

```cpp
int value = 0;

Property<int> score(
    [&]() { return value; },
    [&](int& newValue) { value = std::clamp(newValue, 0, 100); }
);

score = 150;  // Clamped to 100
```

Reactive properties allow:

* Custom getter logic
* Custom setter logic
* Validation and transformation
* Seamless assignment syntax

---

## 🔒 Read-Only Properties

```cpp
int versionValue = 1;
ReadonlyProperty<int> Version([&]() { return versionValue; });

std::cout << Version << "\n"; // 1
// Version = 2; // ❌ Compile error
```

Read-only properties expose values safely without allowing modification.

---

## 📚 ObservableCollection

`ObservableCollection<T>` wraps a `std::vector<T>` and raises:

* `CollectionChanging`
* `CollectionChanged`

It also provides a read-only `Count` property.

```cpp
ObservableCollection<std::string> items;

items.CollectionChanged.Subscribe([](auto&, auto args) {
    std::cout << "New count: " << args.NewCount << "\n";
});

items.push_back("Coffee");
items.push_back("Tea");
items.erase(items.begin());
```

---

### 🔍 ReadonlyObservableCollection

`ReadonlyObservableCollection<T>` provides a read-only view over an existing `ObservableCollection<T>`.

It supports:

* Iteration
* Indexing
* `Count` property
* Change event subscription

But prevents all modifications at compile time.

```cpp
ObservableCollection<int> numbers;
ReadonlyObservableCollection<int> readonlyView(numbers);

readonlyView.CollectionChanged().Subscribe([](auto&, auto args) {
    std::cout << "Collection changed: "
              << args.NewCount << " items\n";
});

numbers.push_back(10);
numbers.push_back(20);

std::cout << "Count: " << readonlyView.Count << "\n";   // 2
std::cout << "First: " << readonlyView.front() << "\n"; // 10

// readonlyView.push_back(30); // ❌ Compile error
// readonlyView.clear();       // ❌ Compile error
```

---

## 📚 Examples

A complete demo application is available in:

```
examples/main.cpp
```

It demonstrates:

* Event subscriptions and scoped subscriptions
* Reactive properties (including read-only patterns)
* `ObservableObject` with property change notifications
* `ObservableCollection` operations (add/remove/insert/clear)
* `ReadonlyObservableCollection` usage
* A real-world shopping cart example

Build and run:

```bash
cmake -B build -S .
cmake --build build
./build/examples/ReactiveLitepp_examples
```

---

## ✅ Tests

The test suite uses Catch2 and is located under `tests/`.

To build and run:

```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake

cmake --build build
cd build && ctest --output-on-failure
```

See `tests/README.md` for additional details.

---

## 📜 License

Licensed under the MIT License.
See `LICENSE` for full details.
