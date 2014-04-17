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
#include "logger/logger.hpp"

namespace logger { class Logger; }

class Router04 : public cSimpleModule {
private:
    std::random_device rd;
    std::default_random_engine engine;
    std::uniform_real_distribution<double> udist_forwardPacketProb;

    int queueLength;
    const static std::size_t QUEUE_NUM = 2;
    std::deque<InfoMessage04 *> queue[QUEUE_NUM];
    DriverMessage04 * pDriverMsg; // this message drives me to send out a message 'msg'

    double alphaProb;
    double betaProb;
    
    // staticstics
    logger::Logger stat_logger;
    int stat_timePassed;
    int stat_packageFrom[2];
    int stat_packageForwardNumber[2];
    int stat_packageCodeNumber;
    int stat_packageLostNumber[2];
    int stat_waitTime;
    double stat_avg_queue_sz[2];
    double stat_avg_queue_all_sz;
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
    /// @brief code a message in the queue on condition is met up, return true if the condition is met up, else false
    bool codeMessageInQueueOnConditionTrue();
    /// @brief code 2 packets in the queues
    InfoMessage04 * codePacket();
    /// @brief wait for packet
    void waitForPacket();
    /// @brief write statistics
    void writeStatistics();
};//class Router04

