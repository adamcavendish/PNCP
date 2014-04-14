#pragma once

// C-STL
#include <cstdio>
#include <cstring>
// STL
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <random>
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N04_NetworkPNCP_m.h"

class Router04 : public cSimpleModule {
private:
    std::default_random_engine engine;
    std::uniform_real_distribution<double> udist_codePacketProb;

    int queueLength;
    const static std::size_t QUEUE_NUM = 2;
    std::deque<InfoMessage04 *> queue[QUEUE_NUM];

    double codePacketProb;

    cMessage * pDriverMsg; // this message drives me to send out a message 'msg'
public:
    Router04();
    virtual ~Router04();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief forward the Message to the corresponding InNode
    void forwardMessage(InfoMessage04 * pInfoMsg);
    /// @brief add message to the corresponding queue
    void addMessageToQueue(InfoMessage04 * pInfoMsg);
    /// @brief broadcast the message to the 2 InNodes
    void broadcastCopyOfMessage(InfoMessage04 * pInfoMsg);
    /// @brief check whether the condition to code packet is met up
    bool codePacketCondition();
    /// @brief check whether the condition to forward the packet is met up
    bool forwardPacketCondition();
    /// @brief forward a message in the queue (forward one message from the queue with maximum length)
    void forwardOneMessageInQueue();
    /// @brief code or wait message in the queue
    void codeOrWaitMessageInQueue();
    /// @brief code 2 packets in the queues
    InfoMessage04 * codePacket();
    /// @brief wait for packet
    void waitForPacket();
};//class Router04

