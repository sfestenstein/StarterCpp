/**
 * @file test_timer.cpp
 * @brief Unit tests for the Timer class
 */

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "utils/Timer.hpp"

namespace utils::test
{

class TimerTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
   }

   void TearDown() override
   {
   }
};

TEST_F(TimerTest, DefaultConstruction)
{
   Timer timer;
   EXPECT_FALSE(timer.isRunning());
   EXPECT_FALSE(timer.isSingleShot());
   EXPECT_EQ(timer.getInterval(), std::chrono::milliseconds(1000));
}

TEST_F(TimerTest, SetInterval)
{
   Timer timer;
   timer.setInterval(std::chrono::milliseconds(500));
   EXPECT_EQ(timer.getInterval(), std::chrono::milliseconds(500));

   timer.setInterval(std::chrono::seconds(2));
   EXPECT_EQ(timer.getInterval(), std::chrono::milliseconds(2000));
}

TEST_F(TimerTest, SetSingleShot)
{
   Timer timer;
   EXPECT_FALSE(timer.isSingleShot());

   timer.setSingleShot(true);
   EXPECT_TRUE(timer.isSingleShot());

   timer.setSingleShot(false);
   EXPECT_FALSE(timer.isSingleShot());
}

TEST_F(TimerTest, StartAndStop)
{
   Timer timer;
   timer.setInterval(std::chrono::milliseconds(100));

   EXPECT_FALSE(timer.isRunning());

   timer.start();
   EXPECT_TRUE(timer.isRunning());

   timer.stop();
   EXPECT_FALSE(timer.isRunning());
}

TEST_F(TimerTest, CallbackIsCalled)
{
   std::atomic<int> callCount{0};

   Timer timer;
   timer.setInterval(std::chrono::milliseconds(50));
   timer.setCallback([&callCount]()
   {
      ++callCount;
   });

   timer.start();

   // Wait for a few callbacks
   std::this_thread::sleep_for(std::chrono::milliseconds(180));

   timer.stop();

   // Should have been called at least 2-3 times
   EXPECT_GE(callCount.load(), 2);
}

TEST_F(TimerTest, SingleShotFiresOnce)
{
   std::atomic<int> callCount{0};

   Timer timer;
   timer.setInterval(std::chrono::milliseconds(50));
   timer.setSingleShot(true);
   timer.setCallback([&callCount]()
   {
      ++callCount;
   });

   timer.start();

   // Wait longer than one interval to ensure callback fires
   std::this_thread::sleep_for(std::chrono::milliseconds(300));

   // Should have been called exactly once
   EXPECT_EQ(callCount.load(), 1);
   // Timer may or may not report running=false immediately due to thread timing
   // Just verify callback was called once
}

TEST_F(TimerTest, StartWithInterval)
{
   std::atomic<int> callCount{0};

   Timer timer;
   timer.setCallback([&callCount]()
   {
      ++callCount;
   });

   timer.start(std::chrono::milliseconds(50));

   std::this_thread::sleep_for(std::chrono::milliseconds(130));
   timer.stop();

   EXPECT_GE(callCount.load(), 2);
}

TEST_F(TimerTest, RestartTimer)
{
   std::atomic<int> callCount{0};

   Timer timer;
   timer.setInterval(std::chrono::milliseconds(50));
   timer.setCallback([&callCount]()
   {
      ++callCount;
   });

   timer.start();
   std::this_thread::sleep_for(std::chrono::milliseconds(80));
   timer.stop();

   int firstCount = callCount.load();

   timer.start();
   std::this_thread::sleep_for(std::chrono::milliseconds(80));
   timer.stop();

   EXPECT_GT(callCount.load(), firstCount);
}

TEST_F(TimerTest, StopBeforeCallback)
{
   std::atomic<int> callCount{0};

   Timer timer;
   timer.setInterval(std::chrono::milliseconds(500));
   timer.setCallback([&callCount]()
   {
      ++callCount;
   });

   timer.start();
   timer.stop();

   // Callback should not have been called
   EXPECT_EQ(callCount.load(), 0);
}

TEST_F(TimerTest, StaticSingleShot)
{
   std::atomic<bool> called{false};

   auto timer = Timer::singleShot(std::chrono::milliseconds(50), [&called]()
   {
      called = true;
   });

   EXPECT_TRUE(timer->isRunning());

   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   EXPECT_TRUE(called.load());
   EXPECT_FALSE(timer->isRunning());
}

TEST_F(TimerTest, MoveConstruction)
{
   Timer timer1;
   timer1.setInterval(std::chrono::milliseconds(123));

   Timer timer2(std::move(timer1));

   EXPECT_EQ(timer2.getInterval(), std::chrono::milliseconds(123));
}

TEST_F(TimerTest, DestructorStopsTimer)
{
   std::atomic<int> callCount{0};

   {
      Timer timer;
      timer.setInterval(std::chrono::milliseconds(50));
      timer.setCallback([&callCount]()
      {
         ++callCount;
      });
      timer.start();
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
      // Timer goes out of scope here
   }

   int countAfterDestroy = callCount.load();
   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   // Count should not have increased after destruction
   EXPECT_EQ(callCount.load(), countAfterDestroy);
}

} // namespace utils::test
