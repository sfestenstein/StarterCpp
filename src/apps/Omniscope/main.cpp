/**
 * @file main.cpp
 * @brief Entry point for the Omniscope application.
 *
 * Usage:
 *   Omniscope [domain_id] [http_port]
 *   Open http://localhost:<port>  (default 8080)
 */

#include "OmniscopeApp.h"
#include "DdsTransport.h"
#include "CommonUtils/GeneralLogger.h"

#include <cstdint>
#include <memory>
#include <string>

int main(int argc, char *argv[])
{
   CommonUtils::GeneralLogger logger;
   logger.init("Omniscope");

   uint32_t domainId = 0;
   uint16_t httpPort = 8080;

   if (argc > 1) domainId = static_cast<uint32_t>(std::stoul(argv[1]));
   if (argc > 2) httpPort = static_cast<uint16_t>(std::stoul(argv[2]));

   Omniscope::OmniscopeApp app(httpPort);
   app.addTransport(std::make_unique<Omniscope::DdsTransport>(domainId));
   app.run();

   return 0;
}
