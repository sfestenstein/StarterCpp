/**
 * @file Timer.cpp
 * @brief Implementation of the Timer class
 */

#include "utils/Timer.hpp"

namespace utils
{

Timer::Timer() = default;

Timer::~Timer()
{
   stop();
}

Timer::Timer(Timer&& other) noexcept
{
   std::lock_guard<std::mutex> lock(other.m_mutex);
   m_callback = std::move(other.m_callback);
   m_interval = other.m_interval;
   m_singleShot.store(other.m_singleShot.load());
   // Note: we don't move the running state or thread
}

Timer& Timer::operator=(Timer&& other) noexcept
{
   if (this != &other)
   {
      stop();
      std::scoped_lock lock(m_mutex, other.m_mutex);
      m_callback = std::move(other.m_callback);
      m_interval = other.m_interval;
      m_singleShot.store(other.m_singleShot.load());
   }
   return *this;
}

void Timer::setInterval(Duration interval)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_interval = interval;
}

Timer::Duration Timer::getInterval() const
{
   std::lock_guard<std::mutex> lock(m_mutex);
   return m_interval;
}

void Timer::setCallback(Callback callback)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_callback = std::move(callback);
}

void Timer::setSingleShot(bool singleShot)
{
   m_singleShot = singleShot;
}

bool Timer::isSingleShot() const
{
   return m_singleShot;
}

void Timer::start()
{
   stop();

   m_stopRequested = false;
   m_running = true;
   m_thread = std::thread(&Timer::timerThread, this);
}

void Timer::start(Duration interval)
{
   setInterval(interval);
   start();
}

void Timer::stop()
{
   m_stopRequested = true;
   {
      std::lock_guard<std::mutex> lock(m_cvMutex);
   }
   m_cv.notify_all();

   if (m_thread.joinable())
   {
      m_thread.join();
   }

   m_running = false;
}

bool Timer::isRunning() const
{
   return m_running;
}

std::shared_ptr<Timer> Timer::singleShot(Duration interval, Callback callback)
{
   auto timer = std::make_shared<Timer>();
   timer->setSingleShot(true);
   timer->setCallback([timer, callback = std::move(callback)]()
   {
      if (callback)
      {
         callback();
      }
   });
   timer->start(interval);
   return timer;
}

void Timer::timerThread()
{
   while (!m_stopRequested)
   {
      Duration interval;
      Callback callback;

      // Copy callback and interval while holding mutex
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         interval = m_interval;
         callback = m_callback;
      }

      // Wait for the interval or until stop is requested
      {
         std::unique_lock<std::mutex> cvLock(m_cvMutex);
         if (m_cv.wait_for(cvLock, interval, [this]()
         {
            return m_stopRequested.load();
         }))
         {
            // Stop was requested
            break;
         }
      }

      // Execute callback if not stopped
      if (!m_stopRequested && callback)
      {
         callback();
      }

      // If single-shot, stop after first execution
      if (m_singleShot && !m_stopRequested)
      {
         m_running = false;
         break;
      }
   }
}

} // namespace utils
