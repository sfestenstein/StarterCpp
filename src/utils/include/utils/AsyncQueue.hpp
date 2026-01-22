#pragma once
/**
 * @file AsyncQueue.hpp
 * @brief Thread-safe asynchronous queue for inter-thread communication
 *
 * Provides a blocking queue implementation suitable for producer-consumer
 * patterns and message passing between threads.
 */

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <chrono>

namespace utils
{

/**
 * @brief Thread-safe asynchronous queue
 *
 * A blocking queue that supports multiple producers and consumers.
 * Provides both blocking and non-blocking operations for pushing
 * and popping elements.
 *
 * @tparam T Type of elements stored in the queue
 *
 * @example
 * @code
 * AsyncQueue<int> queue;
 *
 * // Producer thread
 * queue.push(42);
 *
 * // Consumer thread
 * auto value = queue.pop(); // Blocks until value available
 * @endcode
 */
template<typename T>
class AsyncQueue
{
public:
   /**
    * @brief Construct a new AsyncQueue object
    * @param maxSize Maximum queue size (0 = unlimited)
    */
   explicit AsyncQueue(std::size_t maxSize = 0)
      : m_maxSize(maxSize)
   {
   }

   /**
    * @brief Destroy the AsyncQueue object
    */
   ~AsyncQueue()
   {
      close();
   }

   // Non-copyable
   AsyncQueue(const AsyncQueue&) = delete;
   AsyncQueue& operator=(const AsyncQueue&) = delete;

   // Movable
   AsyncQueue(AsyncQueue&& other) noexcept
   {
      std::lock_guard<std::mutex> lock(other.m_mutex);
      m_queue = std::move(other.m_queue);
      m_maxSize = other.m_maxSize;
      m_closed = other.m_closed.load();
   }

   AsyncQueue& operator=(AsyncQueue&& other) noexcept
   {
      if (this != &other)
      {
         std::scoped_lock lock(m_mutex, other.m_mutex);
         m_queue = std::move(other.m_queue);
         m_maxSize = other.m_maxSize;
         m_closed = other.m_closed.load();
      }
      return *this;
   }

   /**
    * @brief Push an element to the queue (blocking if full)
    * @param value Value to push
    * @return true if pushed successfully, false if queue is closed
    */
   bool push(const T& value)
   {
      std::unique_lock<std::mutex> lock(m_mutex);

      // Wait if queue is full
      if (m_maxSize > 0)
      {
         m_cvNotFull.wait(lock, [this]()
         {
            return m_queue.size() < m_maxSize || m_closed;
         });
      }

      if (m_closed)
      {
         return false;
      }

      m_queue.push(value);
      lock.unlock();
      m_cvNotEmpty.notify_one();
      return true;
   }

   /**
    * @brief Push an element to the queue (move version)
    * @param value Value to push (will be moved)
    * @return true if pushed successfully, false if queue is closed
    */
   bool push(T&& value)
   {
      std::unique_lock<std::mutex> lock(m_mutex);

      if (m_maxSize > 0)
      {
         m_cvNotFull.wait(lock, [this]()
         {
            return m_queue.size() < m_maxSize || m_closed;
         });
      }

      if (m_closed)
      {
         return false;
      }

      m_queue.push(std::move(value));
      lock.unlock();
      m_cvNotEmpty.notify_one();
      return true;
   }

   /**
    * @brief Try to push an element without blocking
    * @param value Value to push
    * @return true if pushed, false if queue is full or closed
    */
   bool tryPush(const T& value)
   {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (m_closed || (m_maxSize > 0 && m_queue.size() >= m_maxSize))
      {
         return false;
      }

      m_queue.push(value);
      m_cvNotEmpty.notify_one();
      return true;
   }

   /**
    * @brief Pop an element from the queue (blocking)
    * @return The popped element, or std::nullopt if queue is closed and empty
    */
   std::optional<T> pop()
   {
      std::unique_lock<std::mutex> lock(m_mutex);

      m_cvNotEmpty.wait(lock, [this]()
      {
         return !m_queue.empty() || m_closed;
      });

      if (m_queue.empty())
      {
         return std::nullopt;
      }

      T value = std::move(m_queue.front());
      m_queue.pop();

      lock.unlock();
      m_cvNotFull.notify_one();

      return value;
   }

   /**
    * @brief Try to pop an element without blocking
    * @return The popped element, or std::nullopt if queue is empty
    */
   std::optional<T> tryPop()
   {
      std::lock_guard<std::mutex> lock(m_mutex);

      if (m_queue.empty())
      {
         return std::nullopt;
      }

      T value = std::move(m_queue.front());
      m_queue.pop();

      m_cvNotFull.notify_one();
      return value;
   }

   /**
    * @brief Pop with timeout
    * @param timeout Maximum time to wait
    * @return The popped element, or std::nullopt if timeout or closed
    */
   template<typename Rep, typename Period>
   std::optional<T> popFor(std::chrono::duration<Rep, Period> timeout)
   {
      std::unique_lock<std::mutex> lock(m_mutex);

      if (!m_cvNotEmpty.wait_for(lock, timeout, [this]()
      {
         return !m_queue.empty() || m_closed;
      }))
      {
         return std::nullopt; // Timeout
      }

      if (m_queue.empty())
      {
         return std::nullopt;
      }

      T value = std::move(m_queue.front());
      m_queue.pop();

      lock.unlock();
      m_cvNotFull.notify_one();

      return value;
   }

   /**
    * @brief Get the current size of the queue
    * @return Number of elements in the queue
    */
   std::size_t size() const
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_queue.size();
   }

   /**
    * @brief Check if the queue is empty
    * @return true if empty, false otherwise
    */
   bool empty() const
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_queue.empty();
   }

   /**
    * @brief Close the queue
    *
    * After closing, no new elements can be pushed.
    * Existing elements can still be popped.
    * Wakes up all waiting threads.
    */
   void close()
   {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_closed = true;
      }
      m_cvNotEmpty.notify_all();
      m_cvNotFull.notify_all();
   }

   /**
    * @brief Check if the queue is closed
    * @return true if closed, false otherwise
    */
   bool isClosed() const
   {
      return m_closed;
   }

   /**
    * @brief Clear all elements from the queue
    */
   void clear()
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      std::queue<T> empty;
      std::swap(m_queue, empty);
      m_cvNotFull.notify_all();
   }

private:
   mutable std::mutex m_mutex;
   std::condition_variable m_cvNotEmpty;
   std::condition_variable m_cvNotFull;
   std::queue<T> m_queue;
   std::size_t m_maxSize;
   std::atomic<bool> m_closed{false};
};

} // namespace utils
