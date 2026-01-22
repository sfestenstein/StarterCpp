# StarterCpp

A robust, production-ready C++20 starter project template with modern build tooling, comprehensive testing, and CI/CD integration.

## Features

- **C++20** standard with modern compiler support
- **CMake 3.25+** build system with presets
- **Conan 2.0** package manager for dependencies
- **Google Test** unit testing framework
- **Protocol Buffers** for serialization
- **ZeroMQ** for messaging
- **spdlog** for logging
- **Code quality tools**: clang-format, clang-tidy
- **Sanitizers**: AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan)
- **Code coverage** with gcov/lcov
- **GitHub Actions** CI/CD pipeline

## Quick Start

### Prerequisites

- **Compiler**: GCC 13+ or Clang 15+ (Linux/macOS)
- **CMake**: 3.25+
- **Conan**: 2.0+
- **Build Tool**: Ninja (recommended) or Make
- **Python**: 3.8+ (for Conan)

### Build Instructions

#### Debug Build (with sanitizers)

```bash
# 1. Clone the repository
git clone https://github.com/yourorg/StarterCpp.git
cd StarterCpp

# 2. Create a Conan profile (one-time setup)
conan profile detect --force

# 3. Install dependencies with Conan
conan install . --output-folder=build/debug --build=missing -s build_type=Debug

# 4. Configure with CMake preset
cmake --preset debug

# 5. Build
cmake --build --preset debug

# 6. Run tests
ctest --preset debug

# 7. Run the applications
./build/debug/bin/publisher   # In terminal 1
./build/debug/bin/subscriber  # In terminal 2
```

#### Release Build

```bash
# Install release dependencies
conan install . --output-folder=build/release --build=missing -s build_type=Release

# Configure and build
cmake --preset release
cmake --build --preset release
```

#### Coverage Build

```bash
# Install dependencies with coverage enabled
conan install . --output-folder=build/coverage --build=missing -s build_type=Debug \
    -o enable_coverage=True

# Configure, build, and generate coverage report
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage
cmake --build --preset coverage --target coverage

# View report in build/coverage/coverage/index.html
```

## Project Structure

```
StarterCpp/
├── src/
│   ├── utils/              # Utility library
│   │   ├── include/utils/  # Public headers
│   │   ├── Logger.cpp      # Logging wrapper
│   │   ├── Timer.cpp       # Timer class
│   │   └── AsyncQueue.cpp  # Thread-safe queue
│   ├── proto/              # Protocol buffer library
│   └── apps/               # Executables
│       ├── publisher_main.cpp
│       └── subscriber_main.cpp
├── proto/                  # Protocol buffer definitions
│   ├── sensor_data.proto
│   ├── commands.proto
│   └── configuration.proto
├── tests/                  # Unit tests
├── docs/                   # Documentation
├── .github/                # GitHub configuration
│   ├── workflows/          # CI/CD pipelines
│   └── copilot-instructions.md
├── CMakeLists.txt          # Root CMake configuration
├── CMakePresets.json       # CMake presets
├── conanfile.py            # Conan package configuration
├── .clang-format           # Code formatting rules
├── .clang-tidy             # Static analysis rules
└── .editorconfig           # Editor configuration
```

## Documentation

- [Project Design](docs/DESIGN.md) - Architecture and design decisions
- [Build Guide](docs/BUILD.md) - Detailed build instructions
- [Development Guide](docs/DEVELOPMENT.md) - Contributing and development workflow

## CMake Presets

The project uses CMake presets for consistent build configurations across different environments.

### Configure Presets

| Preset | Build Type | Tests | Coverage | Sanitizers | Use Case |
|--------|------------|-------|----------|------------|----------|
| `debug` | Debug | ✅ | ❌ | ✅ | Local development |
| `release` | Release | ❌ | ❌ | ❌ | Production builds |
| `coverage` | Debug | ✅ | ✅ | ❌ | Code coverage reports |
| `ci-linux` | Debug | ✅ | ✅ | ❌ | GitHub Actions CI |

### Using Presets

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset debug

# Build with a preset
cmake --build --preset debug

# Test with a preset
ctest --preset debug
```

### Custom Configuration (without presets)

```bash
# Manual configuration example
cmake -B build/custom -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=build/custom/conan_toolchain.cmake \
    -DBUILD_TESTS=ON \
    -DENABLE_SANITIZERS=ON
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS` | ON | Build unit tests |
| `ENABLE_COVERAGE` | OFF | Enable code coverage |
| `ENABLE_SANITIZERS` | OFF | Enable ASan/UBSan |
| `ENABLE_CLANG_TIDY` | OFF | Enable clang-tidy |

## Dependencies

Managed by Conan 2.0:

| Package | Version | Purpose |
|---------|---------|---------|
| spdlog | 1.13.0 | Logging |
| protobuf | 5.27.0 | Serialization |
| zeromq | 4.3.5 | Messaging |
| cppzmq | 4.10.0 | C++ ZeroMQ bindings |
| gtest | 1.14.0 | Unit testing |

## License

This project is provided as a starter template. Add your own license as needed.
