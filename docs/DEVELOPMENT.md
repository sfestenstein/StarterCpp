# Development Guide

This document covers the development workflow, coding standards, and contribution guidelines for the StarterCpp project.

## Development Environment Setup

### Recommended IDE

**Visual Studio Code** with these extensions:
- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- clangd (LLVM)
- GitHub Copilot
- Error Lens

### VS Code Configuration

Create `.vscode/settings.json`:
```json
{
   "cmake.configureOnOpen": true,
   "cmake.buildDirectory": "${workspaceFolder}/build/debug",
   "cmake.configureSettings": {
      "CMAKE_TOOLCHAIN_FILE": "${workspaceFolder}/build/debug/conan_toolchain.cmake"
   },
   "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
   "editor.formatOnSave": true,
   "clang-format.executable": "clang-format",
   "[cpp]": {
      "editor.defaultFormatter": "xaver.clang-format"
   }
}
```

## Workflow

### 1. Create a Feature Branch

```bash
git checkout -b feature/my-feature
```

### 2. Make Changes

Follow the coding standards below.

### 3. Format Code

```bash
# Format all source files
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

### 4. Run Static Analysis

```bash
# Rebuild with clang-tidy enabled
cmake --preset debug -DENABLE_CLANG_TIDY=ON
cmake --build --preset debug
```

### 5. Run Tests

```bash
ctest --preset debug --output-on-failure
```

### 6. Check Coverage

```bash
cmake --preset coverage
cmake --build --preset coverage
cmake --build --preset coverage --target coverage
```

### 7. Commit and Push

```bash
git add .
git commit -m "feat: add my feature"
git push origin feature/my-feature
```

## Coding Standards

### File Organization

- Headers: `src/<lib>/include/<lib>/<Header>.hpp`
- Sources: `src/<lib>/<Source>.cpp`
- Tests: `tests/test_<source>.cpp`

### Naming Conventions

| Element | Style | Example |
|---------|-------|---------|
| Classes/Structs | PascalCase | `SensorReading` |
| Functions/Methods | camelCase | `processData()` |
| Variables | camelCase | `sensorValue` |
| Member Variables | m_ prefix | `m_value` |
| Static Members | s_ prefix | `s_instance` |
| Constants | UPPER_CASE | `MAX_RETRIES` |
| Namespaces | lower_case | `common_utils` |
| Files | PascalCase | `AsyncQueue.hpp` |

### Code Style

```cpp
// Use braces on new line (Allman style)
void processData()
{
   if (condition)
   {
      // 3-space indentation
      doSomething();
   }
}

// Use m_ prefix for member variables
class MyClass
{
public:
   void setValue(int value)
   {
      m_value = value;
   }

private:
   int m_value;
};

// Use explicit types or auto with care
auto result = calculateValue();  // OK when type is obvious
std::vector<int> numbers;        // Prefer explicit for clarity

// Use nullptr, not NULL or 0
if (pointer == nullptr)
{
   // ...
}

// Use override for virtual methods
class Derived : public Base
{
public:
   void virtualMethod() override;
};
```

### Header Guidelines

```cpp
#pragma once  // Use pragma once, not include guards

// Include order:
// 1. Corresponding header (for .cpp files)
// 2. Project headers
// 3. Third-party headers
// 4. System headers

#include "MyClass.hpp"

#include "CommonUtils/GeneralLogger.h"
#include "proto/messages.pb.h"

#include <spdlog/spdlog.h>
#include <zmq.hpp>

#include <memory>
#include <string>
#include <vector>
```

### Documentation

```cpp
/**
 * @brief Short description of the function
 *
 * Longer description if needed.
 *
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * @throws ExceptionType When this exception is thrown
 *
 * @example
 * @code
 * auto result = myFunction(1, "test");
 * @endcode
 */
int myFunction(int param1, std::string_view param2);
```

## Adding New Components

### Adding a New CommonUtils Class

1. Create header: `src/CommonUtils/NewClass.h`
2. Create source: `src/CommonUtils/NewClass.cpp`
3. Add to `src/CommonUtils/CMakeLists.txt`:
   ```cmake
   add_library(common_utils STATIC
      ...
      NewClass.cpp
   )
   ```
4. Create test: `tests/CommonUtilsTests/NewClassUt.cpp`
5. Add to `tests/CommonUtilsTests/CMakeLists.txt`:
   ```cmake
   add_executable(CommonUtilsTests
      ...
      NewClassUt.cpp
   )
   ```

### Adding a New Proto File

1. Create proto: `proto/new_message.proto`
2. Add to `src/proto/CMakeLists.txt`:
   ```cmake
   set(PROTO_FILES
      ...
      ${CMAKE_SOURCE_DIR}/proto/new_message.proto
   )
   ```

### Adding a New Application

1. Create source: `src/apps/new_app_main.cpp`
2. Add to `src/apps/CMakeLists.txt`:
   ```cmake
   add_executable(new_app
      new_app_main.cpp
   )

   target_link_libraries(new_app
      PRIVATE
         StarterCpp::common_utils
         # other dependencies
   )
   ```

## Testing Guidelines

### Test Organization

```cpp
// Use Test Fixtures for common setup
class MyClassTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      // Common setup
   }

   void TearDown() override
   {
      // Common cleanup
   }

   MyClass m_instance;
};

// Name tests descriptively
TEST_F(MyClassTest, MethodName_Condition_ExpectedResult)
{
   // Arrange
   // ...

   // Act
   auto result = m_instance.method();

   // Assert
   EXPECT_EQ(result, expected);
}
```

### Test Coverage Goals

- Aim for 80%+ line coverage
- Cover all public methods
- Test edge cases and error conditions
- Test thread safety for concurrent classes

## Commit Message Format

Use conventional commits:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Code restructuring
- `test`: Adding tests
- `chore`: Maintenance

Examples:
```
feat(common_utils): add snooze capability to Timer

fix(timer): prevent race condition on stop

docs(readme): update build instructions
```

## CI/CD Pipeline

The GitHub Actions workflow:

1. **Build**: Compiles on Linux and Windows (MinGW)
2. **Test**: Runs all unit tests
3. **Coverage**: Generates and uploads coverage reports
4. **Quality**: Checks code formatting

All checks must pass before merging.
