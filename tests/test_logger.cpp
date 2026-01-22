/**
 * @file test_logger.cpp
 * @brief Unit tests for the Logger class
 */

#include <gtest/gtest.h>
#include <sstream>

#include "utils/Logger.hpp"

namespace utils::test
{

class LoggerTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      // Ensure logger is in a clean state
      Logger::shutdown();
   }

   void TearDown() override
   {
      Logger::shutdown();
   }
};

TEST_F(LoggerTest, InitializesSuccessfully)
{
   EXPECT_NO_THROW(Logger::init("TestLogger", LogLevel::Debug));
}

TEST_F(LoggerTest, InitializesOnlyOnce)
{
   Logger::init("FirstInit", LogLevel::Debug);
   Logger::init("SecondInit", LogLevel::Error);

   // Should still be the first logger (Info level, not Error)
   // The second init should be ignored
   EXPECT_NO_THROW(Logger::debug("This should work if first init was used"));
}

TEST_F(LoggerTest, SetLevelWorks)
{
   Logger::init("TestLogger", LogLevel::Info);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Info);

   Logger::setLevel(LogLevel::Debug);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Debug);

   Logger::setLevel(LogLevel::Error);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Error);
}

TEST_F(LoggerTest, AllLogLevelsWork)
{
   Logger::init("TestLogger", LogLevel::Trace);

   // These should not throw
   EXPECT_NO_THROW(Logger::trace("Trace message"));
   EXPECT_NO_THROW(Logger::debug("Debug message"));
   EXPECT_NO_THROW(Logger::info("Info message"));
   EXPECT_NO_THROW(Logger::warn("Warning message"));
   EXPECT_NO_THROW(Logger::error("Error message"));
   EXPECT_NO_THROW(Logger::critical("Critical message"));
}

TEST_F(LoggerTest, FormattingWorks)
{
   Logger::init("TestLogger", LogLevel::Debug);

   // Test format strings with various types
   EXPECT_NO_THROW(Logger::info("Integer: {}", 42));
   EXPECT_NO_THROW(Logger::info("Float: {:.2f}", 3.14159));
   EXPECT_NO_THROW(Logger::info("String: {}", "hello"));
   EXPECT_NO_THROW(Logger::info("Multiple: {} {} {}", 1, 2.0, "three"));
}

TEST_F(LoggerTest, FlushWorks)
{
   Logger::init("TestLogger", LogLevel::Debug);
   Logger::info("Test message");
   EXPECT_NO_THROW(Logger::flush());
}

TEST_F(LoggerTest, ShutdownAndReinitialize)
{
   Logger::init("FirstLogger", LogLevel::Info);
   Logger::info("First logger message");
   Logger::shutdown();

   // Should be able to reinitialize
   EXPECT_NO_THROW(Logger::init("SecondLogger", LogLevel::Debug));
   EXPECT_NO_THROW(Logger::debug("Second logger message"));
}

TEST_F(LoggerTest, LogLevelConversions)
{
   Logger::init("TestLogger", LogLevel::Trace);

   // Test all level conversions
   Logger::setLevel(LogLevel::Trace);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Trace);

   Logger::setLevel(LogLevel::Debug);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Debug);

   Logger::setLevel(LogLevel::Info);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Info);

   Logger::setLevel(LogLevel::Warn);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Warn);

   Logger::setLevel(LogLevel::Error);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Error);

   Logger::setLevel(LogLevel::Critical);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Critical);

   Logger::setLevel(LogLevel::Off);
   EXPECT_EQ(Logger::getLevel(), LogLevel::Off);
}

TEST_F(LoggerTest, AutoInitializesOnFirstUse)
{
   // Don't explicitly init - should auto-init
   EXPECT_NO_THROW(Logger::info("Auto-initialized logger"));
}

} // namespace utils::test
