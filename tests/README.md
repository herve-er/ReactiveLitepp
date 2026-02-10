# ReactiveLitepp Test Suite

Comprehensive test suite for the ReactiveLitepp library using Catch2.

## Prerequisites

- CMake 3.14 or higher
- vcpkg (for Catch2 dependency management)
- C++17 compatible compiler

## Setup with vcpkg

### 1. Install vcpkg (if not already installed)

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat  # On Windows
./bootstrap-vcpkg.sh   # On Linux/Mac
```

### 2. Configure CMake with vcpkg toolchain

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
```

Or set the environment variable:
```bash
export CMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
```

### 3. Build the project

```bash
cmake --build build
```

### 4. Run the tests

```bash
cd build
ctest --output-on-failure
```

Or run the test executable directly:
```bash
./build/tests/ReactiveLitepp_tests
```

## Test Categories

The test suite is organized into four main categories:

### 1. Basic Functionality Tests (`test_event_basic.cpp`)
- Single and multiple handler subscriptions
- Unsubscribe behavior
- Multiple argument types
- ScopedSubscription RAII behavior
- Subscription validity checking
- Mixed subscription types
- Exception handling

### 2. Multi-threading Tests (`test_event_multithreading.cpp`)
- Concurrent subscriptions from multiple threads
- Concurrent notifications
- Subscribe and notify simultaneously
- Concurrent unsubscribe operations
- Race condition testing
- Subscription destruction during notifications
- Comprehensive stress test with multiple threads

### 3. Lifetime & Memory Tests (`test_event_lifetime.cpp`)
- Dangling event pointers
- Subscriptions outliving events
- Events in smart pointers (shared_ptr, unique_ptr)
- Subscription cleanup
- Circular reference handling
- Multiple events sharing handlers
- Self-unsubscribing handlers
- Destruction order scenarios

### 4. Stress Tests (`test_event_stress.cpp`)
- Many handlers (10,000+)
- Many events (1,000+)
- Rapid subscribe/unsubscribe cycles
- Random operations
- Deeply nested event chains
- Memory usage under load
- Large payload handling
- Subscription churn
- Performance benchmarks

## Running Specific Tests

### Run only basic tests
```bash
./build/tests/ReactiveLitepp_tests "[event][basic]"
```

### Run only multithreading tests
```bash
./build/tests/ReactiveLitepp_tests "[event][multithreading]"
```

### Run only lifetime tests
```bash
./build/tests/ReactiveLitepp_tests "[event][lifetime]"
```

### Run stress tests (excluding benchmarks)
```bash
./build/tests/ReactiveLitepp_tests "[event][stress]" -e "[benchmark]"
```

### Run performance benchmarks
```bash
./build/tests/ReactiveLitepp_tests "[benchmark]"
```

## Test Coverage

The test suite covers:

? **Thread Safety**
- Race conditions
- Concurrent access patterns
- Memory ordering

? **Memory Management**
- No memory leaks
- Dangling pointer prevention
- Weak pointer usage
- Smart pointer compatibility

? **API Correctness**
- All public methods
- Edge cases
- Error conditions
- RAII behavior

? **Performance**
- Scalability with many handlers
- Subscription overhead
- Notification latency

## Continuous Integration

To run tests in CI:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
cd build && ctest --output-on-failure
```

## Troubleshooting

### Catch2 not found
Ensure vcpkg is properly configured and the toolchain file is specified in CMake configuration.

### Tests fail to build
Check that you have C++17 support enabled in your compiler.

### Multithreading tests are flaky
Some timing-sensitive tests may occasionally fail on heavily loaded systems. Re-run the tests or increase timeout values.

## Contributing

When adding new features to ReactiveLitepp, please add corresponding tests in the appropriate test file.
