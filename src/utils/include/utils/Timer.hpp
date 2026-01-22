#pragma once
/**
 * @file Timer.hpp
 * @brief Timer class similar to QTimer for periodic and single-shot callbacks
 *
 * Provides a simple timer implementation that can execute callbacks
 * either once (single-shot) or repeatedly at specified intervals.
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace utils
{

/**
 * @brief A timer class similar to Qt's QTimer
 *
 * Supports both single-shot and repeating timers with millisecond precision.
 * The timer runs in its own thread and invokes the callback when the
 * specified interval elapses.
 *
 * @example
 * @code
 * Timer timer;
 * timer.setInterval(std::chrono::seconds(1));
 * timer.setCallback([]() { std::cout << "Tick!" << std::endl; });
 * timer.start();
 * // ... later
 * timer.stop();
 * @endcode
 */
class Timer
{
public:
   using Callback = std::function<void()>;
   using Duration = std::chrono::milliseconds;

   /**
    * @brief Construct a new Timer object
    */
   Timer();

   /**
    * @brief Destroy the Timer object
    *
    * Automatically stops the timer if running.
    */
   ~Timer();

   // Non-copyable
   Timer(const Timer&) = delete;
   Timer& operator=(const Timer&) = delete;

   // Movable
   Timer(Timer&& other) noexcept;
   Timer& operator=(Timer&& other) noexcept;

   /**
    * @brief Set the timer interval
    * @param interval Duration between timer events
    */
   void setInterval(Duration interval);

   /**
    * @brief Get the current timer interval
    * @return Current interval duration
    */
   Duration getInterval() const;

   /**
    * @brief Set the callback function to be called on timer events
    * @param callback Function to call when timer fires
    */
   void setCallback(Callback callback);

   /**
    * @brief Set whether the timer is single-shot
    * @param singleShot If true, timer fires once and stops
    */
   void setSingleShot(bool singleShot);

   /**
    * @brief Check if the timer is single-shot
    * @return true if single-shot, false if repeating
    */
   bool isSingleShot() const;

   /**
    * @brief Start the timer
    *
    * If the timer is already running, it will be restarted.
    */
   void start();

   /**
    * @brief Start the timer with a new interval
    * @param interval New interval duration
    */
   void start(Duration interval);

   /**
    * @brief Stop the timer
    */
   void stop();

   /**
    * @brief Check if the timer is currently running
    * @return true if running, false otherwise
    */
   bool isRunning() const;

   /**
    * @brief Create and start a single-shot timer
    * @param interval Duration before the callback is invoked
    * @param callback Function to call when timer fires
    * @return Shared pointer to the created timer
    */
   static std::shared_ptr<Timer> singleShot(Duration interval, Callback callback);

private:
   void timerThread();

   mutable std::mutex m_mutex;         ///< Mutex for protecting m_callback and m_interval
   mutable std::mutex m_cvMutex;       ///< Mutex for condition variable wait
   std::condition_variable m_cv;
   std::thread m_thread;
   Callback m_callback;
   Duration m_interval{1000};
   std::atomic<bool> m_running{false};
   std::atomic<bool> m_singleShot{false};
   std::atomic<bool> m_stopRequested{false};
};

} // namespace utils
