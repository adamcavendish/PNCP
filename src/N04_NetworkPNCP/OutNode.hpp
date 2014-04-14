#pragma once

// STL
#include <random>
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N04_NetworkPNCP_m.h"

class OutNode04 : public cSimpleModule {
private:
    std::default_random_engine engine;
    std::uniform_real_distribution<double> udist_generatePacketProb;
    int counter;           // counter is the number of packets to send
    double generatePacketProb;       // generatePacketProb is the send probability of every timeslot
    cMessage * pDriverMsg; // this message drives me to send out a message 'msg'
    InfoMessage04 * pMsg;  // the real message send to other nodes
public:
    OutNode04();
    virtual ~OutNode04();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief check whether OutNode is going to send out a message
    bool checkSendMessage();
    /// @brief generate a message for OutNode to send to InNode
    InfoMessage04 * generateMessage();
};//class OutNode04

