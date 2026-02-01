# StarterCpp

A robust, production-ready C++20 starter project template with modern build tooling, comprehensive testing, and CI/CD integration.

## Features

- **C++20** standard with modern compiler support
- **CMake 3.25+** build system with presets
- **Conan 2.0** package manager for dependencies
- **Google Test** unit testing framework
- **Protocol Buffers** for serialization
- **ZeroMQ** for messaging (cppzmq)
- **Zyre** for peer-to-peer discovery and messaging
- **CZMQ** high-level C binding for ZeroMQ
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

#### 1. Clone and Setup

```bash
# Clone the repository
git clone https://github.com/yourorg/StarterCpp.git
cd StarterCpp

# Create a Conan profile (one-time setup)
conan profile detect --force
```

#### 2. Install Dependencies (One-Time)

Install both Debug and Release configurations to a unified build folder:

```bash
conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.cppstd=20
```

This installs all dependencies for all presets (debug, release, coverage, ci-linux).

#### 3. Build and Test

**Debug Build** (with sanitizers and tests):
```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

**Release Build** (optimized):
```bash
cmake --preset release
cmake --build --preset release
```

**Coverage Build**:
```bash
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage
cmake --build --preset coverage --target CommonUtilsCoverage
# View report in build/coverage/CommonUtilsCoverage/index.html
```

#### 4. Run Applications

```bash
# Zyre-based pub/sub (peer-to-peer discovery)
./build/debug/bin/ZyreSubscriber  # In terminal 1
./build/debug/bin/ZyrePublisher   # In terminal 2

# High-bandwidth UDP multicast pub/sub
./build/debug/bin/HighBandwidthSubscriber  # In terminal 1
./build/debug/bin/HighBandwidthPublisher   # In terminal 2
```

## Project Structure

```
StarterCpp/
├── src/
│   ├── apps/                     # Executables
│   │   ├── ZyrePublisherTest.cpp
│   │   ├── ZyreSubscriberTest.cpp
│   │   ├── HighBandwidthPublisherTester.cpp
│   │   └── HighBandwidthSubscriberTester.cpp
│   └── libs/                     # Libraries
│       ├── CommonUtils/          # Common utilities library
│       │   ├── GeneralLogger.h       # Async logging wrapper (spdlog)
│       │   ├── Timer.h               # Basic timer class
│       │   ├── SnoozableTimer.h      # Timer with snooze capability
│       │   └── DataHandler.h         # Data handling utilities
│       ├── PubSub/               # Publish-Subscribe library
│       │   ├── ZyreNode.h            # Base Zyre node class
│       │   ├── ZyrePublisher.h       # Zyre-based publisher
│       │   ├── ZyreSubscriber.h      # Zyre-based subscriber
│       │   ├── HighBandwidthPublisher.h   # UDP multicast publisher
│       │   └── HighBandwidthSubscriber.h  # UDP multicast subscriber
│       └── proto/                # Protocol buffer library
│           └── proto-messages/       # Protocol buffer definitions
│               ├── sensor_data.proto
│               ├── commands.proto
│               └── configuration.proto
├── tests/                        # Unit tests
│   ├── CommonUtilsTests/         # CommonUtils unit tests
│   └── PubSubTests/              # PubSub unit tests
├── docs/                         # Documentation
├── .github/                      # GitHub configuration
│   ├── workflows/                # CI/CD pipelines
│   └── copilot-instructions.md
├── CMakeLists.txt                # Root CMake configuration
├── CMakePresets.json             # CMake presets
├── conanfile.py                  # Conan package configuration
├── .clang-format                 # Code formatting rules
├── .clang-tidy                   # Static analysis rules
└── .editorconfig                 # Editor configuration
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
# Manual configuration example (using unified Conan output)
cmake -B build/custom -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=build/build/Debug/generators/conan_toolchain.cmake \
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
| spdlog | 1.15.0 | Logging |
| protobuf | 5.27.0 | Serialization |
| zeromq | 4.3.5 | Low-level messaging |
| cppzmq | 4.10.0 | C++ ZeroMQ bindings |
| czmq | 4.2.1 | High-level C ZeroMQ binding |
| zyre | 2.0.1 | Peer-to-peer discovery |
| gtest | 1.14.0 | Unit testing |

## License

This project is provided as a starter template. Add your own license as needed.
