#ifndef OMNISCOPEAPP_H_
#define OMNISCOPEAPP_H_

#include "ITransport.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Omniscope
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
class OmniscopeApp
{
public:
   OmniscopeApp(std::vector<std::unique_ptr<ITransport>> transports,
                 uint16_t httpPortArg);
   ~OmniscopeApp();

   OmniscopeApp(const OmniscopeApp &) = delete;
   OmniscopeApp &operator=(const OmniscopeApp &) = delete;

   /// Run the event loop.  Blocks until the global stop signal is raised.
   void run();

private:
   struct Impl;
   std::unique_ptr<Impl> _impl;
};

} // namespace Omniscope

#endif // OMNISCOPEAPP_H_
