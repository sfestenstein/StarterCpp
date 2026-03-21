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
- **Omniscope** — web-based traffic inspector for any pub/sub transport (DDS, Zyre, ZMQ, …)
- **VITA 49.2** signal data and context packet codec
- **Crow** for embedded HTTP/WebSocket server
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

# Omniscope (web-based traffic inspector)
./build/debug/bin/Omniscope             # Default: domain 0, port 8080
./build/debug/bin/Omniscope 1 9090      # Domain 1, port 9090
# Open http://localhost:8080 in a browser

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
| cyclonedds | 0.10.5 | DDS middleware |
| cyclonedds-cxx | 0.10.5 | C++ DDS bindings |
| crowcpp-crow | 1.3.1 | HTTP/WebSocket server |
| gtest | 1.14.0 | Unit testing |

## Omniscope

Omniscope is a browser-based traffic inspector for any pub/sub transport in the project. It ships with a DDS transport out of the box and is designed to be extended with Zyre, ZMQ, or custom transports.

### Quick Start

```bash
# Start the monitor (default: DDS domain 0, port 8080)
./build/debug/bin/Omniscope

# With custom domain ID and port
./build/debug/bin/Omniscope 1 9090
```

Open **http://localhost:8080** in a browser.

### Features

- **Live topic subscription** — click a topic in the left pane to subscribe; messages stream in real-time via WebSocket
- **3-pane UI** — Topics | Messages | Detail with dark-theme monospace interface
- **Recording** — click Record to capture messages to a `.ddsrec` (JSON Lines) file
- **Playback** — load a `.ddsrec` file and replay it with original timing (capped at 5 s per gap), republishing to DDS so other subscribers see the data
- **Progress indicator** — thin accent progress bar and footer percentage during playback
- **Multi-transport architecture** — `ITransport` interface allows plugging in new transports without changing the monitor core

### Architecture

The monitor lives in `src/apps/Omniscope/` and is structured around a clean transport abstraction:

| File | Purpose |
|------|---------|
| `ITransport.h` | Abstract transport interface |
| `TransportDds.h/.cpp` | Eclipse Cyclone DDS transport (pImpl) |
| `PlaybackEngine.h/.cpp` | Recording load + threaded playback |
| `OmniscopeApp.h/.cpp` | Crow HTTP/WebSocket orchestrator (pImpl) |
| `CrowCompat.h` | C++20 / libc++ compatibility shim for Crow |
| `main.cpp` | Entry point |
| `web/monitor.html` | Embedded single-page UI |

### Adding a New Transport

1. Create a class that implements `Omniscope::ITransport` (see `TransportDds` for reference)
2. Instantiate it in `main.cpp` and push it into the transports vector
3. Rebuild — the monitor automatically discovers topics from all registered transports

```cpp
auto zyreTransport = std::make_unique<Omniscope::ZyreTransport>(/* ... */);
transports.push_back(std::move(zyreTransport));
```

## License

This project is licensed under the [MIT License](LICENSE).
