# Project Design

This document describes the architecture and design decisions of the StarterCpp project.

## Overview

StarterCpp is designed as a production-ready C++ project template that demonstrates modern C++ best practices, build system configuration, and software engineering patterns. It includes multiple messaging libraries (Zyre, UDP multicast, DDS), a web-based IPC traffic monitor, a VITA 49.2 signal data codec, and Protocol Buffers for serialization.

## Architecture

### Component Diagram

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                              Applications                                     │
│  ┌──────────────┐ ┌──────────────┐ ┌───────────────┐ ┌───────────────────┐   │
│  │ ZyrePublisher│ │ZyreSubscriber│ │ HighBandwidth │ │  DDS Publisher /  │   │
│  │              │ │              │ │   Pub / Sub   │ │  DDS Subscriber   │   │
│  └──────┬───────┘ └──────┬───────┘ └──────┬────────┘ └────────┬──────────┘   │
│         │                │                │                   │              │
│  ┌──────────────────┐  ┌────────────────────┐  ┌─────────────────────────┐   │
│  │ Vita49RoundTrip  │  │ Vita49PerfBenchmark│  │    Vita49FileCodec     │   │
│  └────────┬─────────┘  └─────────┬──────────┘  └───────────┬─────────────┘   │
│           │                      │                         │                 ││  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                        Omniscope                                         │   │
│  │  ┌───────────────┐  ┌───────────────┐  ┌─────────────────────────────┐     │   │
│  │  │ OmniscopeApp  │  │PlaybackEngine│  │ ITransport (DDS, Zyre…) │     │   │
│  │  │ (Crow server) │  │ (recording)  │  │ DdsTransport            │     │   │
│  │  └───────────────┘  └───────────────┘  └─────────────────────────────┘     │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
│                                                                               │├───────────┴──────────────────────┴─────────────────────────┴─────────────────┤
│                                Libraries                                      │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │                          PubSub Library                                  │  │
│  │  ┌───────────────┐  ┌───────────────┐  ┌─────────────────────────────┐  │  │
│  │  │   ZyreNode    │  │ ZyrePublisher │  │ HighBandwidthPublisher      │  │  │
│  │  │   (base)      │  │ ZyreSubscriber│  │ HighBandwidthSubscriber     │  │  │
│  │  └───────────────┘  └───────────────┘  └─────────────────────────────┘  │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │                        CycloneDDS Library (INTERFACE)                     │  │
│  │  ┌────────────────┐  ┌────────────────┐  ┌─────────────────────────┐    │  │
│  │  │ DDSTopicConfig │  │ DDSPublisher<T>│  │ DDSSubscriber<T>        │    │  │
│  │  │ (QoS registry) │  │ (header-only)  │  │ (header-only, polling)  │    │  │
│  │  └────────────────┘  └────────────────┘  └─────────────────────────┘    │  │
│  │  ┌────────────────────────────────────────┐                              │  │
│  │  │ DDSMessages (IDL-generated C++ types)  │                              │  │
│  │  └────────────────────────────────────────┘                              │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │                        Vita49_2 Library                                   │  │
│  │  ┌──────────────┐  ┌──────────────────┐  ┌─────────────────────────┐    │  │
│  │  │ PacketHeader │  │ SignalDataPacket │  │ ContextPacket           │    │  │
│  │  │              │  │                  │  │                         │    │  │
│  │  └──────────────┘  └──────────────────┘  └─────────────────────────┘    │  │
│  │  ┌──────────────┐  ┌──────────────────┐                                  │  │
│  │  │ Vita49Codec  │  │ ByteSwap        │                                  │  │
│  │  └──────────────┘  └──────────────────┘                                  │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│  ┌─────────────────┐                    ┌─────────────────┐                   │
│  │   Proto Lib     │                    │  CommonUtils    │                   │
│  │   (protobuf)    │                    │  (utilities)    │                   │
│  └─────────────────┘                    └─────────────────┘                   │
│                                                                               │
├───────────────────────────────────────────────────────────────────────────────┤
│                          External Dependencies                                │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐  │
│  │ spdlog │ │protobuf│ │ ZeroMQ │ │  CZMQ  │ │  Zyre  │ │Cyclone │ │  Crow  │  │
│  │        │ │        │ │ cppzmq │ │        │ │        │ │  DDS   │ │        │  │
│  └────────┘ └────────┘ └────────┘ └────────┘ └────────┘ └────────┘ └────────┘  │
└───────────────────────────────────────────────────────────────────────────────┘
```

### Libraries

#### CommonUtils Library (`src/libs/CommonUtils/`)

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

#### PubSub Library (`src/libs/PubSub/`)

The PubSub library provides two messaging patterns:

**Zyre-based Messaging** (peer-to-peer discovery):

- **ZyreNode**: Base class for Zyre nodes:
  - Automatic peer discovery via UDP beaconing
  - Node lifecycle management (start/stop)
  - Thread-safe operation

- **ZyrePublisher**: Publishes messages via Zyre:
  - Inherits from ZyreNode
  - Publishes protobuf messages to topics
  - Automatic peer discovery

- **ZyreSubscriber**: Subscribes to messages via Zyre:
  - Inherits from ZyreNode
  - Topic-based subscription with callbacks
  - Background receive thread

**High-Bandwidth Messaging** (UDP multicast):

- **HighBandwidthPublisher**: Fast UDP multicast publisher:
  - Raw UDP multicast for minimal overhead
  - Automatic message fragmentation for large payloads
  - Fire-and-forget semantics (unreliable but fast)
  - Ideal for sensor data, telemetry, video frames

- **HighBandwidthSubscriber**: Fast UDP multicast subscriber:
  - Joins multicast group for receiving
  - Automatic fragment reassembly
  - Configurable reassembly timeout
  - Thread-safe subscription (can subscribe before or after start)

#### Proto Library (`src/libs/proto/`)

The protocol buffer library compiles `.proto` files from `src/libs/proto/proto-messages/` into C++ classes:

- **sensor_data.proto**: Sensor readings with metadata, location, and batching
- **commands.proto**: Command/response pattern for RPC
- **configuration.proto**: Application configuration structures

#### CycloneDDS Library (`src/libs/CycloneDDS/`)

The CycloneDDS library provides topic-based DDS publish-subscribe via Eclipse Cyclone DDS. All wrapper classes are header-only; the CMake target (`CycloneDDSLib`) is an INTERFACE library.

- **DDSTopicConfig**: Central registry mapping topic names to `DataWriterQos` and `DataReaderQos`, guaranteeing RxO (Request-vs-Offered) compatibility between publishers and subscribers.

- **DDSPublisher\<T\>**: Template publisher that lazily creates `DataWriter` instances per topic. QoS is looked up from `DDSTopicConfig` automatically.

- **DDSSubscriber\<T\>**: Template subscriber with a background polling thread. Subscribes to topics with user callbacks; reader QoS is looked up from `DDSTopicConfig`.

- **DDSMessages**: IDL-generated C++ types from `idl/` directory (SensorData, Command, TrackData), compiled via `IDLCXX_GENERATE()`.

#### Vita49_2 Library (`src/libs/Vita49_2/`)

The Vita49_2 library implements the VITA 49.2 standard for signal data and context packets:

- **PacketHeader**: Parses and serializes VITA 49 packet headers (packet type, class/stream identifiers, size)
- **SignalDataPacket**: Signal data packet codec (IF data, extension data)
- **ContextPacket**: Context packet codec (metadata about signal acquisition parameters)
- **Vita49Codec**: High-level codec facade for encoding/decoding complete VITA 49 packets
- **Vita49Types**: Common type definitions and enumerations for the VITA 49 standard
- **ByteSwap**: Byte-order utilities for network/host conversion

### Applications

#### ZyrePublisher (`src/apps/TestApps/ZyrePublisherTest.cpp`)

Demonstrates:
- Zyre peer-to-peer publishing
- Protocol buffer serialization (SensorReading, SensorDataBatch, Command)
- Periodic message publishing
- GeneralLogger usage

#### ZyreSubscriber (`src/apps/TestApps/ZyreSubscriberTest.cpp`)

Demonstrates:
- Zyre peer-to-peer subscription
- Protocol buffer deserialization
- Topic-based message handling
- Formatted logging with spdlog

#### HighBandwidthPublisher (`src/apps/TestApps/HighBandwidthPublisherTester.cpp`)

Demonstrates:
- High-frequency UDP multicast publishing
- Large message fragmentation
- Sensor data streaming

#### HighBandwidthSubscriber (`src/apps/TestApps/HighBandwidthSubscriberTester.cpp`)

Demonstrates:
- UDP multicast subscription
- Fragment reassembly
- High-throughput message reception

#### DDSPublisher (`src/apps/TestApps/DDSPublisherTest.cpp`)

Demonstrates:
- Eclipse Cyclone DDS topic-based publishing
- DDSTopicConfig with Reliable and BestEffort QoS
- Publishing IDL-defined SensorReading and TrackUpdate messages

#### DDSSubscriber (`src/apps/TestApps/DDSSubscriberTest.cpp`)

Demonstrates:
- Eclipse Cyclone DDS topic-based subscription
- Shared DDSTopicConfig for QoS alignment with publisher
- Callback-based message handling with background polling

#### Omniscope (`src/apps/Omniscope/`)

A web-based traffic inspector for IPC pub/sub transports:
- **Transport abstraction** — `ITransport` interface decouples the monitor from any specific middleware; ships with `DdsTransport` and is extensible to Zyre, ZMQ, etc.
- **OmniscopeApp** — orchestrates transports, Crow HTTP/WebSocket server, recording, and playback (pImpl pattern)
- **PlaybackEngine** — loads `.ddsrec` files and replays them in a background thread with original inter-message timing (capped at 5 s per gap), publishing back through the transport
- **DdsTransport** — concrete transport using Eclipse Cyclone DDS; pImpl hides all DDS headers
- **CrowCompat.h** — C++20 / libc++ compatibility shim (atomic `operator<<`) for Crow 1.3.x
- **Embedded HTML UI** — dark-theme 3-pane interface (Topics / Messages / Detail) served at `/`, with WebSocket streaming, recording controls, load/playback with progress bar

Usage:
```bash
./build/debug/bin/Omniscope [domain_id] [http_port]
# Open http://localhost:8080
```

#### Vita49RoundTripTest (`src/apps/TestApps/Vita49RoundTripTest.cpp`)

Demonstrates:
- Round-trip encode/decode of VITA 49.2 signal data packets
- Verification of codec correctness

#### Vita49PerfBenchmark (`src/apps/Tools/Vita49PerfBenchmark.cpp`)

Demonstrates:
- Performance benchmarking of VITA 49.2 codec operations

#### Vita49FileCodec (`src/apps/Tools/Vita49FileCodec.cpp`)

Demonstrates:
- File-based VITA 49.2 packet generation, inspection, and round-trip testing

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
