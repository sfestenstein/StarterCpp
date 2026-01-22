/**
 * @file test_async_queue.cpp
 * @brief Unit tests for the AsyncQueue class
 */

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <future>
#include <string>
#include <thread>
#include <vector>

#include "utils/AsyncQueue.hpp"

namespace utils::test
{

class AsyncQueueTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
   }

   void TearDown() override
   {
   }
};

TEST_F(AsyncQueueTest, DefaultConstruction)
{
   AsyncQueue<int> queue;
   EXPECT_TRUE(queue.empty());
   EXPECT_EQ(queue.size(), 0);
   EXPECT_FALSE(queue.isClosed());
}

TEST_F(AsyncQueueTest, PushAndPop)
{
   AsyncQueue<int> queue;

   queue.push(42);
   EXPECT_FALSE(queue.empty());
   EXPECT_EQ(queue.size(), 1);

   auto value = queue.pop();
   EXPECT_TRUE(value.has_value());
   EXPECT_EQ(*value, 42);
   EXPECT_TRUE(queue.empty());
}

TEST_F(AsyncQueueTest, PushMultiple)
{
   AsyncQueue<int> queue;

   for (int i = 0; i < 5; ++i)
   {
      queue.push(i);
   }

   EXPECT_EQ(queue.size(), 5);

   for (int i = 0; i < 5; ++i)
   {
      auto value = queue.pop();
      EXPECT_TRUE(value.has_value());
      EXPECT_EQ(*value, i);
   }

   EXPECT_TRUE(queue.empty());
}

TEST_F(AsyncQueueTest, TryPushAndTryPop)
{
   AsyncQueue<int> queue;

   EXPECT_TRUE(queue.tryPush(42));
   EXPECT_EQ(queue.size(), 1);

   auto value = queue.tryPop();
   EXPECT_TRUE(value.has_value());
   EXPECT_EQ(*value, 42);

   // Try to pop from empty queue
   auto emptyValue = queue.tryPop();
   EXPECT_FALSE(emptyValue.has_value());
}

TEST_F(AsyncQueueTest, MoveValue)
{
   AsyncQueue<std::string> queue;

   std::string str = "Hello, World!";
   queue.push(std::move(str));

   auto value = queue.pop();
   EXPECT_TRUE(value.has_value());
   EXPECT_EQ(*value, "Hello, World!");
}

TEST_F(AsyncQueueTest, MaxSizeBlocking)
{
   AsyncQueue<int> queue(2); // Max size 2

   queue.push(1);
   queue.push(2);
   EXPECT_EQ(queue.size(), 2);

   // Try push should fail when full
   EXPECT_FALSE(queue.tryPush(3));

   // Pop one
   queue.pop();
   EXPECT_EQ(queue.size(), 1);

   // Now try push should succeed
   EXPECT_TRUE(queue.tryPush(3));
}

TEST_F(AsyncQueueTest, Close)
{
   AsyncQueue<int> queue;
   queue.push(42);

   EXPECT_FALSE(queue.isClosed());

   queue.close();

   EXPECT_TRUE(queue.isClosed());

   // Push should fail when closed
   EXPECT_FALSE(queue.push(100));

   // But existing items can still be popped
   auto value = queue.pop();
   EXPECT_TRUE(value.has_value());
   EXPECT_EQ(*value, 42);

   // Pop on empty closed queue returns nullopt
   auto emptyValue = queue.pop();
   EXPECT_FALSE(emptyValue.has_value());
}

TEST_F(AsyncQueueTest, Clear)
{
   AsyncQueue<int> queue;

   for (int i = 0; i < 10; ++i)
   {
      queue.push(i);
   }

   EXPECT_EQ(queue.size(), 10);

   queue.clear();

   EXPECT_TRUE(queue.empty());
   EXPECT_EQ(queue.size(), 0);
}

TEST_F(AsyncQueueTest, PopForTimeout)
{
   AsyncQueue<int> queue;

   auto start = std::chrono::steady_clock::now();
   auto value = queue.popFor(std::chrono::milliseconds(100));
   auto end = std::chrono::steady_clock::now();

   auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

   EXPECT_FALSE(value.has_value());
   EXPECT_GE(elapsed.count(), 90); // Allow some tolerance
}

TEST_F(AsyncQueueTest, PopForSuccess)
{
   AsyncQueue<int> queue;

   // Push in a separate thread after a delay
   std::thread producer([&queue]()
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      queue.push(42);
   });

   auto value = queue.popFor(std::chrono::milliseconds(200));

   producer.join();

   EXPECT_TRUE(value.has_value());
   EXPECT_EQ(*value, 42);
}

TEST_F(AsyncQueueTest, ConcurrentProducerConsumer)
{
   AsyncQueue<int> queue;
   const int numItems = 100;
   std::atomic<int> consumed{0};

   // Producer thread
   std::thread producer([&queue, numItems]()
   {
      for (int i = 0; i < numItems; ++i)
      {
         queue.push(i);
      }
      queue.close();
   });

   // Consumer thread
   std::thread consumer([&queue, &consumed]()
   {
      while (true)
      {
         auto value = queue.pop();
         if (!value.has_value())
         {
            break;
         }
         ++consumed;
      }
   });

   producer.join();
   consumer.join();

   EXPECT_EQ(consumed.load(), numItems);
}

TEST_F(AsyncQueueTest, MultipleProducersMultipleConsumers)
{
   AsyncQueue<int> queue;
   const int numProducers = 4;
   const int numConsumers = 4;
   const int itemsPerProducer = 100;
   std::atomic<int> produced{0};
   std::atomic<int> consumed{0};

   std::vector<std::thread> producers;
   std::vector<std::thread> consumers;

   // Start consumers
   for (int i = 0; i < numConsumers; ++i)
   {
      consumers.emplace_back([&queue, &consumed]()
      {
         while (true)
         {
            auto value = queue.popFor(std::chrono::milliseconds(100));
            if (!value.has_value() && queue.isClosed())
            {
               break;
            }
            if (value.has_value())
            {
               ++consumed;
            }
         }
      });
   }

   // Start producers
   for (int i = 0; i < numProducers; ++i)
   {
      producers.emplace_back([&queue, &produced, itemsPerProducer]()
      {
         for (int j = 0; j < itemsPerProducer; ++j)
         {
            queue.push(j);
            ++produced;
         }
      });
   }

   // Wait for producers
   for (auto& p : producers)
   {
      p.join();
   }

   // Signal consumers to stop
   queue.close();

   // Wait for consumers
   for (auto& c : consumers)
   {
      c.join();
   }

   EXPECT_EQ(produced.load(), numProducers * itemsPerProducer);
   EXPECT_EQ(consumed.load(), numProducers * itemsPerProducer);
}

TEST_F(AsyncQueueTest, MoveConstruction)
{
   AsyncQueue<int> queue1;
   queue1.push(1);
   queue1.push(2);

   AsyncQueue<int> queue2(std::move(queue1));

   EXPECT_EQ(queue2.size(), 2);
}

TEST_F(AsyncQueueTest, MoveAssignment)
{
   AsyncQueue<int> queue1;
   queue1.push(1);
   queue1.push(2);

   AsyncQueue<int> queue2;
   queue2 = std::move(queue1);

   EXPECT_EQ(queue2.size(), 2);
}

TEST_F(AsyncQueueTest, PopWakesUpOnClose)
{
   AsyncQueue<int> queue;

   std::future<std::optional<int>> future = std::async(std::launch::async, [&queue]()
   {
      return queue.pop();
   });

   std::this_thread::sleep_for(std::chrono::milliseconds(50));

   queue.close();

   auto result = future.wait_for(std::chrono::milliseconds(100));
   EXPECT_EQ(result, std::future_status::ready);

   auto value = future.get();
   EXPECT_FALSE(value.has_value());
}

} // namespace utils::test
