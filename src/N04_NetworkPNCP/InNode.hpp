#pragma once

// STL
#include <queue>
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N04_NetworkPNCP_m.h"

class InNode04 : public cSimpleModule {
private:
    std::deque<InfoMessage04 *> queue;
public:
    InNode04();
    virtual ~InNode04();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief filter the message to see whether the message is sent to me.
    void filterMessage(InfoMessage04 * pInfoMsg);
    /// @brief after check the message is sent to me, 'useMessage()' 'uses' the message and delete it
    void useMessage(InfoMessage04 * pInfoMsg);
    /// @brief when the message is not sent to me, add it to the queue for decode
    void addMessageToQueue(InfoMessage04 * pInfoMsg);
    /// @brief decode the message. 'pInfoMsg' is the coded message
    InfoMessage04 * decodePacket(InfoMessage04 * pInfoMsg);
};//class InNode04

