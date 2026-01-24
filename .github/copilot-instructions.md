# GitHub Copilot Instructions for StarterCpp

This document provides context and guidelines for GitHub Copilot when working with the StarterCpp project.

## Project Overview

StarterCpp is a C++20 starter project template using:
- **Build System**: CMake 3.25+ with presets
- **Package Manager**: Conan 2.0
- **Compiler**: GCC 13+ or Clang 15+ (Linux/macOS)
- **Testing**: Google Test
- **Dependencies**: spdlog, protobuf, ZeroMQ (cppzmq)

## Project Structure

```
StarterCpp/
├── src/
│   ├── utils/              # Utility library (Logger, Timer, AsyncQueue)
│   │   └── include/utils/  # Public headers
│   ├── proto/              # Protocol buffer library
│   │   └── proto-messages/ # .proto source files
│   └── apps/               # Executables (publisher, subscriber)
├── tests/                  # Unit tests
├── docs/                   # Documentation
└── .github/                # CI/CD and this file
```

## Coding Conventions

### Style Guide

- **Indentation**: 3 spaces (no tabs)
- **Braces**: Allman style (opening brace on new line)
- **Line length**: 100 characters maximum
- **Naming**:
  - Classes: `PascalCase`
  - Functions/Methods: `camelCase`
  - Variables: `camelCase`
  - Member variables: `m_` prefix (e.g., `m_value`)
  - Static members: `s_` prefix (e.g., `s_instance`)
  - Constants: `UPPER_CASE`
  - Namespaces: `lower_case`

### Code Example

```cpp
namespace utils
{

class MyClass
{
public:
   MyClass();

   void processData(int value);
   int getValue() const;

private:
   void internalMethod();

   int m_value;
   static int s_counter;
};

} // namespace utils
```

### Header Structure

```cpp
#pragma once

// Corresponding header (for .cpp files)
// Project headers
// Third-party headers
// System headers

#include "utils/Logger.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>
```

## Common Tasks

### Adding a New Utility Class

1. Create `src/utils/include/utils/NewClass.hpp`
2. Create `src/utils/NewClass.cpp`
3. Add `NewClass.cpp` to `src/utils/CMakeLists.txt`
4. Create `tests/test_new_class.cpp`
5. Add test file to `tests/CMakeLists.txt`

### Adding a New Proto Message

1. Create or edit file in `src/proto/proto-messages/` directory
2. Proto files are auto-discovered via glob in `src/proto/CMakeLists.txt`
3. Include generated header as `#include "message_name.pb.h"`

### Adding a New Application

1. Create `src/apps/new_app_main.cpp`
2. Add to `src/apps/CMakeLists.txt`:
   ```cmake
   add_executable(new_app new_app_main.cpp)
   target_link_libraries(new_app PRIVATE StarterCpp::utils ...)
   ```

## Build Commands

```bash
# Install dependencies
conan install . --output-folder=build/debug --build=missing -s build_type=Debug

# Configure
cmake --preset debug -DCMAKE_TOOLCHAIN_FILE=build/debug/conan_toolchain.cmake

# Build
cmake --build --preset debug

# Test
ctest --preset debug

# Coverage
cmake --build --preset coverage --target coverage
```

## CMake Targets

- `utils` - Utility library (alias: `StarterCpp::utils`)
- `proto_lib` - Protobuf library (alias: `StarterCpp::proto`)
- `publisher` - ZeroMQ publisher application
- `subscriber` - ZeroMQ subscriber application
- `unit_tests` - All unit tests

## Dependencies Available

When suggesting code, these libraries are available:

| Library | Include | Namespace/Usage |
|---------|---------|-----------------|
| spdlog | `<spdlog/spdlog.h>` | `spdlog::info()` or `utils::Logger` |
| protobuf | `"message.pb.h"` | `messages::MessageType` |
| ZeroMQ | `<zmq.hpp>` | `zmq::context_t`, `zmq::socket_t` |
| Google Test | `<gtest/gtest.h>` | `TEST()`, `EXPECT_EQ()` |

## Testing Patterns

```cpp
#include <gtest/gtest.h>
#include "utils/MyClass.hpp"

namespace utils::test
{

class MyClassTest : public ::testing::Test
{
protected:
   void SetUp() override { }
   void TearDown() override { }

   MyClass m_instance;
};

TEST_F(MyClassTest, MethodName_Condition_ExpectedResult)
{
   // Arrange
   // Act
   // Assert
   EXPECT_EQ(actual, expected);
}

} // namespace utils::test
```

## Error Handling Patterns

- Use exceptions for recoverable errors
- Use `std::optional` for values that may not exist
- Log errors using `utils::Logger::error()`
- Use RAII for resource management

## Thread Safety

- Use `std::mutex` with `std::lock_guard` or `std::unique_lock`
- Use `std::atomic` for simple flags/counters
- The `AsyncQueue` class provides a thread-safe queue implementation

## Important Notes

1. **No raw pointers for ownership** - Use `std::unique_ptr` or `std::shared_ptr`
2. **Prefer `std::string_view`** for read-only string parameters
3. **Use `[[nodiscard]]`** for functions whose return value should not be ignored
4. **Mark destructors `override`** in derived classes
5. **Use `= default`/`= delete`** for special member functions

## File Modification Guidelines

When modifying files:
- Always run clang-format before committing
- Add corresponding unit tests for new functionality
- Update documentation if adding public API
- Follow existing patterns in the codebase
