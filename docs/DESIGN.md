# Project Design

This document describes the architecture and design decisions of the StarterCpp project.

## Overview

StarterCpp is designed as a production-ready C++ project template that demonstrates modern C++ best practices, build system configuration, and software engineering patterns.

## Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      Applications                            │
│  ┌─────────────────┐           ┌─────────────────┐          │
│  │   Publisher     │           │   Subscriber    │          │
│  │   (ZeroMQ)      │           │   (ZeroMQ)      │          │
│  └────────┬────────┘           └────────┬────────┘          │
│           │                              │                   │
├───────────┼──────────────────────────────┼───────────────────┤
│           │         Libraries            │                   │
│  ┌────────▼────────┐           ┌────────▼────────┐          │
│  │   Proto Lib     │           │  CommonUtils   │          │
│  │   (protobuf)    │           │  (utilities)   │          │
│  └─────────────────┘           └─────────────────┘          │
│                                                              │
├──────────────────────────────────────────────────────────────┤
│                    External Dependencies                     │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │  spdlog  │ │ protobuf │ │  ZeroMQ  │ │  GTest   │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘        │
└──────────────────────────────────────────────────────────────┘
```

### Libraries

#### CommonUtils Library (`src/CommonUtils/`)

The CommonUtils library provides reusable components for common tasks:

- **GeneralLogger**: An async logging wrapper around spdlog providing:
  - Dual-logger system (general + trace)
  - Convenience macros (GPCRIT, GPERROR, GPWARN, GPINFO, GPDEBUG, GPTRACE)
  - Async logging with configurable queue size
  - Thread-safe initialization

- **Timer**: A basic timer class:
  - Periodic and single-shot modes
  - Callback-based design
  - Thread-safe start/stop operations
  - Millisecond precision

- **SnoozableTimer**: An extended timer with snooze capability:
  - Inherits from Timer
  - Snooze functionality to extend timeout
  - Useful for implementing watchdog patterns

- **DataHandler**: Data handling utilities (header-only):
  - Template-based data processing
  - Flexible data transformation support

#### Proto Library (`src/proto/`)

The protocol buffer library compiles `.proto` files from `src/proto/proto-messages/` into C++ classes:

- **sensor_data.proto**: Sensor readings with metadata
- **commands.proto**: Command/response pattern for RPC
- **configuration.proto**: Application configuration structures

### Applications

#### Publisher (`src/apps/publisher_main.cpp`)

Demonstrates:
- Timer usage for periodic events
- Protocol buffer serialization
- ZeroMQ PUB socket pattern
- Graceful signal handling

#### Subscriber (`src/apps/subscriber_main.cpp`)

Demonstrates:
- ZeroMQ SUB socket pattern
- Protocol buffer deserialization
- Formatted logging with spdlog
- Timeout-based message receiving

## Design Decisions

### Build System

**CMake** was chosen as the build system because:
- Industry standard for C++ projects
- Excellent IDE integration
- Cross-platform support
- Modern features (presets, toolchain files)

**Conan 2.0** was chosen for package management because:
- Mature ecosystem with many packages
- First-class CMake integration
- Cross-platform support
- Binary package caching

### Code Quality

**clang-format** ensures consistent code style:
- 3-space indentation
- Allman brace style (braces on new line)
- 100-character line limit

**clang-tidy** provides static analysis:
- Modern C++ best practices
- Bug detection
- Performance suggestions
- Naming conventions

### Testing Strategy

**Google Test** was chosen because:
- Widely used in industry
- Feature-rich (fixtures, mocking, parameterized tests)
- Good IDE integration
- Clear test output

Test organization:
- One test file per source file
- Tests mirror the source structure
- Fixtures for common setup/teardown

### Sanitizers

Address Sanitizer (ASan) and Undefined Behavior Sanitizer (UBSan) are enabled in debug builds to catch:
- Memory leaks
- Buffer overflows
- Use-after-free
- Undefined behavior

### Code Coverage

Coverage is collected using gcov/lcov:
- Line and branch coverage
- HTML report generation
- CI integration with Codecov

## Future Considerations

Areas for potential enhancement:

1. **Benchmarking**: Add Google Benchmark for performance testing
2. **Documentation**: Add Doxygen for API documentation
3. **Packaging**: Add CPack for installers/packages
4. **Cross-compilation**: Add toolchain files for embedded targets
5. **Fuzzing**: Add libFuzzer for fuzz testing
