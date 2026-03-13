# StarterCpp

A robust, production-ready C++20 starter project template with modern build tooling, comprehensive testing, and CI/CD integration.

## License

This project is licensed under the [MIT License](LICENSE).

## Features

- **C++20** standard with modern compiler support
- **CMake 3.25+** build system with presets
- **Conan 2.0** package manager for dependencies
- **Google Test** unit testing framework
- **Protocol Buffers** for serialization
- **ZeroMQ** for messaging (cppzmq)
- **Zyre** for peer-to-peer discovery and messaging
- **CZMQ** high-level C binding for ZeroMQ
- **Eclipse Cyclone DDS** for topic-based DDS publish-subscribe with IDL-defined types and QoS
- **VITA 49.2** signal data and context packet codec
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

> **Note:** Windows (MinGW) is not currently supported due to Conan + MinGW + ZMQ build issues.

### Build Instructions

#### 1. Clone and Setup

```bash
git clone https://github.com/yourorg/StarterCpp.git
cd StarterCpp

# Create a Conan profile (one-time setup)
conan profile detect --force
```

#### 2. Install Dependencies

```bash
conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.cppstd=20
```

If `conan install` fails while building `libsystemd/255` with an error about
"Unknown filesystems defined in kernel headers" (for example `BCACHEFS_SUPER_MAGIC`),
this is typically a mismatch between newer Linux kernel headers and the base `255` recipe.
This project works around it by overriding to a newer `libsystemd/255.x` patch release
in `conanfile.py`.

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

# (Optional) Bind to a specific network interface:
#   ./build/debug/bin/ZyreSubscriber en0
#   ./build/debug/bin/ZyrePublisher  192.168.1.42

# High-bandwidth UDP multicast pub/sub
./build/debug/bin/HighBandwidthSubscriber  # In terminal 1
./build/debug/bin/HighBandwidthPublisher   # In terminal 2

# DDS pub/sub (Eclipse Cyclone DDS, topic-based with QoS)
./build/debug/bin/DDSSubscriber  # In terminal 1
./build/debug/bin/DDSPublisher   # In terminal 2

# VITA 49.2 utilities
./build/debug/bin/Vita49RoundTripTest
./build/debug/bin/Vita49PerfBenchmark
./build/debug/bin/Vita49FileCodec
```

## Documentation

- [Project Design](docs/DESIGN.md) — Architecture and design decisions
- [Build Guide](docs/BUILD.md) — Detailed build instructions
- [Development Guide](docs/DEVELOPMENT.md) — Contributing and development workflow

## CMake Presets

| Preset | Build Type | Tests | Coverage | Sanitizers | Clang-Tidy | Use Case |
|--------|------------|-------|----------|------------|------------|----------|
| `debug` | Debug | Yes | No | Yes | Yes | Local development |
| `release` | Release | No | No | No | No | Production builds |
| `coverage` | Debug | Yes | Yes | No | Yes | Code coverage reports |
| `ci-linux` | Debug | Yes | Yes | No | No | GitHub Actions CI (Linux) |
| `ci-macos` | Debug | Yes | No | No | No | GitHub Actions CI (macOS) |

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
