/**
 * @file main.cpp
 * @brief Entry point for the IPC Monitor application.
 *
 * Usage:
 *   IPCMonitor [domain_id] [http_port]
 *   Open http://localhost:<port>  (default 8080)
 */

#include "IPCMonitorApp.h"
#include "DdsTransport.h"
#include "CommonUtils/GeneralLogger.h"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
   CommonUtils::GeneralLogger logger;
   logger.init("IPCMonitor");

   uint32_t domainId = 0;
   uint16_t httpPort = 8080;

   if (argc > 1) domainId = static_cast<uint32_t>(std::stoul(argv[1]));
   if (argc > 2) httpPort = static_cast<uint16_t>(std::stoul(argv[2]));

   // Build the transport list — add more transports here in the future
   std::vector<std::unique_ptr<IPCMonitor::ITransport>> transports;
   transports.push_back(std::make_unique<IPCMonitor::DdsTransport>(domainId));

   IPCMonitor::IPCMonitorApp app(std::move(transports), httpPort);
   app.run();

   return 0;
}
