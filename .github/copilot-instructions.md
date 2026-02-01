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
│   ├── CommonUtils/          # Common utility library
│   │   ├── GeneralLogger.h   # Async spdlog wrapper with macros
│   │   ├── GeneralLogger.cpp
│   │   ├── Timer.h           # Basic timer class
│   │   ├── Timer.cpp
│   │   ├── SnoozableTimer.h  # Timer with snooze capability
│   │   ├── SnoozableTimer.cpp
│   │   └── DataHandler.h     # Data handling (header-only)
│   ├── proto/              # Protocol buffer library
│   │   └── proto-messages/ # .proto source files
│   └── apps/               # Executables (publisher, subscriber)
├── tests/                  # Unit tests
│   └── CommonUtilsTests/   # Tests for CommonUtils library
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
  - Member variables: `_` prefix (e.g., `_value`)
  - Static members: `s_` prefix (e.g., `s_instance`)

### Code Example

```cpp
namespace CommonUtils
{

class MyClass
{
public:
   MyClass();

   void processData(int value);
   int getValue() const;

private:
   void internalMethod();

   int _value;
   static int s_counter;
};

} // namespace CommonUtils
```

### Header Structure

```cpp
#ifndef MYCLASS_H_
#define MYCLASS_H_

// Project headers
// Third-party headers
// System headers

#include "CommonUtils/GeneralLogger.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

// ... class definition ...

#endif // MYCLASS_H_
```

## Common Tasks

### Adding a New CommonUtils Class

1. Create `src/CommonUtils/NewClass.h`
2. Create `src/CommonUtils/NewClass.cpp` (auto-discovered via `file(GLOB)`)
3. Create `tests/CommonUtilsTests/NewClassUt.cpp` (auto-discovered via `file(GLOB)`)
4. Re-run CMake configure to pick up new files

### Adding a New Proto Message

1. Create or edit file in `src/proto/proto-messages/` directory
2. Proto files are auto-discovered via glob in `src/proto/CMakeLists.txt`
3. Include generated header as `#include "message_name.pb.h"`

### Adding a New Application

1. Create `src/apps/new_app_main.cpp`
2. Add to `src/apps/CMakeLists.txt`:
   ```cmake
   add_executable(new_app new_app_main.cpp)
   target_link_libraries(new_app PRIVATE StarterCpp::common_utils ...)
   ```

## Build Commands

```bash
# Install dependencies (both configurations to unified folder)

conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.cppstd=20

# Configure
cmake --preset debug

# Build
cmake --build --preset debug

# Test
ctest --preset debug

# Coverage
cmake --build --preset coverage --target CommonUtilsCoverage
```

## CMake Targets

- `CommonUtils` - CommonUtils shared library
- `proto_lib` - Protobuf library (alias: `StarterCpp::proto`)
- `publisher` - ZeroMQ publisher application
- `subscriber` - ZeroMQ subscriber application
- `CommonUtilsTests` - CommonUtils unit tests
- `CommonUtilsCoverage` - Coverage report target (when `ENABLE_COVERAGE=ON`)

## Dependencies Available

When suggesting code, these libraries are available:

| Library | Include | Namespace/Usage |
|---------|---------|-----------------|
| spdlog | `<spdlog/spdlog.h>` | `spdlog::info()` or `common_utils::GeneralLogger` |
| protobuf | `"message.pb.h"` | `messages::MessageType` |
| ZeroMQ | `<zmq.hpp>` | `zmq::context_t`, `zmq::socket_t` |
| Google Test | `<gtest/gtest.h>` | `TEST()`, `EXPECT_EQ()` |

## Testing Patterns

```cpp
#include <gtest/gtest.h>
#include "MyClass.h"

// Test naming: TestSuiteName, TestName
TEST(MyClassTest, MethodName_Condition_ExpectedResult)
{
   // Arrange
   CommonUtils::MyClass instance;

   // Act
   auto result = instance.doSomething();

   // Assert
   EXPECT_EQ(result, expected);
}

// For tests needing setup/teardown, use fixtures:
class MyClassFixture : public ::testing::Test
{
protected:
   void SetUp() override { }
   void TearDown() override { }

   CommonUtils::MyClass m_instance;
};

TEST_F(MyClassFixture, MethodName_WithFixture_ExpectedResult)
{
   EXPECT_TRUE(m_instance.isValid());
}
```

## Error Handling Patterns

- Use exceptions for recoverable errors
- Use `std::optional` for values that may not exist
- Log errors using `GPERROR()` macro from GeneralLogger
- Use RAII for resource management

## Thread Safety

- Use `std::mutex` with `std::lock_guard` or `std::unique_lock`
- Use `std::atomic` for simple flags/counters
- The `SnoozableTimer` class provides thread-safe snooze functionality
- The `GeneralLogger` provides thread-safe async logging

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
