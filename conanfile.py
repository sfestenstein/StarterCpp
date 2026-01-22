"""
StarterCpp Conan 2.0 Configuration

This file defines all external dependencies managed by Conan package manager.
"""

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class StarterCppConan(ConanFile):
   name = "startercpp"
   version = "1.0.0"
   description = "A robust C++ starter project template"

   # Build settings
   settings = "os", "compiler", "build_type", "arch"

   # Build options
   options = {
      "shared": [True, False],
      "fPIC": [True, False],
      "build_tests": [True, False],
      "enable_coverage": [True, False],
      "enable_sanitizers": [True, False],
   }

   default_options = {
      "shared": False,
      "fPIC": True,
      "build_tests": True,
      "enable_coverage": False,
      "enable_sanitizers": False,
   }

   # Dependencies
   def requirements(self):
      # Logging
      self.requires("spdlog/1.13.0")

      # Protocol Buffers
      self.requires("protobuf/5.27.0")

      # ZeroMQ messaging
      self.requires("zeromq/4.3.5")
      self.requires("cppzmq/4.10.0")

   def build_requirements(self):
      # Unit testing (only needed during build)
      if self.options.build_tests:
         self.test_requires("gtest/1.14.0")

   def configure(self):
      # Handle fPIC for shared libraries
      if self.options.shared:
         self.options.rm_safe("fPIC")

   def layout(self):
      # Use standard CMake layout
      cmake_layout(self)

   def generate(self):
      # Generate CMake dependency files
      deps = CMakeDeps(self)
      deps.generate()

      # Generate CMake toolchain file
      tc = CMakeToolchain(self)

      # Pass options to CMake
      tc.variables["BUILD_TESTS"] = self.options.build_tests
      tc.variables["ENABLE_COVERAGE"] = self.options.enable_coverage
      tc.variables["ENABLE_SANITIZERS"] = self.options.enable_sanitizers

      tc.generate()
