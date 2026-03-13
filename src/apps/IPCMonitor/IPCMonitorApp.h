#ifndef IPCMONITORAPP_H_
#define IPCMONITORAPP_H_

#include "ITransport.h"
#include "PlaybackEngine.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace IPCMonitor
{

/**
 * @brief Web-based IPC traffic monitor application.
 *
 * Orchestrates one or more ITransport instances, a Crow HTTP/WebSocket
 * server, message recording, and the PlaybackEngine.
 *
 * Workflow:
 *   1. Construct with transports and an HTTP port.
 *   2. Call run() — blocks until interrupted (SIGINT / SIGTERM).
 *   3. Browser clients connect at http://localhost:<port>.
 */
class IPCMonitorApp
{
public:
   IPCMonitorApp(std::vector<std::unique_ptr<ITransport>> transports,
                 uint16_t httpPort);
   ~IPCMonitorApp();

   IPCMonitorApp(const IPCMonitorApp &) = delete;
   IPCMonitorApp &operator=(const IPCMonitorApp &) = delete;

   /// Run the event loop.  Blocks until the global stop signal is raised.
   void run();

private:
   struct Impl;
   std::unique_ptr<Impl> _impl;
};

} // namespace IPCMonitor

#endif // IPCMONITORAPP_H_
