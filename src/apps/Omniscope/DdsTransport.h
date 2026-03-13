#ifndef DDSTRANSPORT_H_
#define DDSTRANSPORT_H_

#include "ITransport.h"

#include <cstdint>
#include <memory>

namespace Omniscope
{

/**
 * @brief DDS transport implementation using Eclipse Cyclone DDS.
 *
 * Subscribes/publishes on DDS topics defined by CycloneDDSConfig.
 * Uses pImpl to keep Cyclone DDS headers out of the public interface.
 */
class DdsTransport : public ITransport
{
public:
   explicit DdsTransport(uint32_t domainId);
   ~DdsTransport() override;

   DdsTransport(const DdsTransport &) = delete;
   DdsTransport &operator=(const DdsTransport &) = delete;

   [[nodiscard]] std::string name() const override;
   [[nodiscard]] std::vector<std::string> topicNames() const override;

   void subscribe(const std::string &topic, MessageCallback callback) override;
   void unsubscribe(const std::string &topic) override;
   [[nodiscard]] bool isSubscribed(const std::string &topic) const override;

   void publishFromJson(const std::string &topic,
                        const std::string &jsonData) override;

private:
   struct Impl;
   std::unique_ptr<Impl> _impl;
};

} // namespace Omniscope

#endif // DDSTRANSPORT_H_
