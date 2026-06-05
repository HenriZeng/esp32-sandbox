#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include "config.h"

class RelayManager {
public:
    RelayManager();
    void initialize();
    void setChannelState(uint8_t channel, bool on);
    void activateNext();
    void reset();
    bool allOperational() const { return currentChannel >= CHANNEL_COUNT; }

private:
    uint8_t currentChannel;
};

#endif