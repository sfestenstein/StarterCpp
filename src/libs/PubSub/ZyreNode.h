#ifndef ZYRELIB_ZYRENODE_H
#define ZYRELIB_ZYRENODE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <zyre.h>


class ZyreNode {
public:
    explicit ZyreNode(const std::string &name);
    virtual ~ZyreNode();

    // start the node; returns true on success
    bool start();
    // request stop (calls zyre_stop)
    void stop();

    [[nodiscard]] const std::string &name() const { return _nodeName; }

protected:
    zyre_t *_node;
    std::string _nodeName;

    // Running flag that derived classes should check
    std::atomic<bool> _isRunning{true};

private:
    // Condition variable for cleanup notification
    std::mutex _terminateMutex;
    std::condition_variable _terminateCV;
    bool _stopRequested{false};
};

#endif //ZYRELIB_ZYRENODE_H