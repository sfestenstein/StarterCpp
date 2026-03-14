#ifndef PLAYBACKENGINE_H_
#define PLAYBACKENGINE_H_

#include "ITransport.h"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Omniscope
{

/**
 * @brief Loads recorded IPC traffic and replays it through a transport.
 *
 * Recorded data is kept in memory so a single recording can be played
 * back multiple times without re-uploading.  Playback honours the
 * original inter-message timing (capped at 5 s per gap).
 */
class PlaybackEngine
{
public:
   using ProgressCallback   = std::function<void(size_t current, size_t total)>;
   using CompletionCallback = std::function<void(bool wasStopped)>;

   explicit PlaybackEngine(ITransport &transport);
   ~PlaybackEngine();

   PlaybackEngine(const PlaybackEngine &) = delete;
   PlaybackEngine &operator=(const PlaybackEngine &) = delete;

   /// Parse a JSON-Lines recording body and store it for playback.
   /// Returns the number of messages loaded.
   size_t loadRecording(const std::string &body);

   /// @return true if a recording is currently loaded.
   [[nodiscard]] bool hasRecording() const;

   /// @return number of messages in the loaded recording.
   [[nodiscard]] size_t messageCount() const;

   /// Start (or restart) playback on a background thread.
   void start(ProgressCallback onProgress, CompletionCallback onComplete);

   /// Signal the playback thread to stop and wait for it to finish.
   void stop();

   /// @return true if playback is currently running.
   [[nodiscard]] bool isPlaying() const;

private:
   ITransport &_transport;

   std::vector<std::string> _lines;
   mutable std::mutex _linesMutex;

   std::thread _thread;
   std::atomic<bool> _stopFlag{false};
   std::atomic<bool> _playing{false};
   std::mutex _stopMutex;
   std::condition_variable _stopCv;
};

} // namespace Omniscope

#endif // PLAYBACKENGINE_H_
