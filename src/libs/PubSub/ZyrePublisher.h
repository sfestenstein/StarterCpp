#ifndef ZYREPUBLISHER_H
#define ZYREPUBLISHER_H

#include "ZyreNode.h"

#include <google/protobuf/message.h>

class ZyrePublisher : public ZyreNode
{
public:
    explicit ZyrePublisher(const std::string &name);
    ~ZyrePublisher();

    // Publish a protobuf message to the specified topic
    // Returns true on success, false on failure
    bool publish(const std::string &topic,
                 const google::protobuf::Message &message);
};

#endif // ZYREPUBLISHER_H
