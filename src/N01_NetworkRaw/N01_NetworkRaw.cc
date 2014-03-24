// C-STL
#include <cstdio>
#include <cstring>
// STL
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N01_NetworkRaw_m.h"
#include "utility/utility.hpp"

class OutNode01 : public cSimpleModule {
private:
    cMessage * pDriverMsg; // this message drives me to send out a message 'msg'
    InfoMessage01 * pMsg; // the real message send to other nodes
public:
    OutNode01();
    virtual ~OutNode01();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    InfoMessage01 * generateMessage();
};

class InNode01 : public cSimpleModule {
private:
    InfoMessage01 * pMsg; // point to the message received
public:
    InNode01();
    virtual ~InNode01();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
};

class Router01 : public cSimpleModule {
public:
    Router01();
    virtual ~Router01();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    void forwardMessage(InfoMessage01 * pInfoMsg);
};

Define_Module(OutNode01);
Define_Module(InNode01);
Define_Module(Router01);

/// --------------------------------------------------
/// implementation part

/// --------------------------------------------------
/// OutNode

OutNode01::OutNode01() {
    pDriverMsg = nullptr;
    pMsg = nullptr;
}// constructor

OutNode01::~OutNode01() {
    if(pDriverMsg)
        delete pDriverMsg;
    if(pMsg)
        delete pMsg;
}// destructor

void OutNode01::initialize() {
    pDriverMsg = new cMessage("");
    scheduleAt(simTime(), pDriverMsg);
}// initialize()

void OutNode01::finish() {
    cancelAndDelete(pDriverMsg);
}//finish()

InfoMessage01 * OutNode01::generateMessage() {
    const int msgStrSize = 64;
    char msgStr[msgStrSize];
    snprintf(msgStr, msgStrSize, "Msg from S%d to D%d", getIndex(), getIndex());

    InfoMessage01 * pTempMsg = new InfoMessage01(msgStr);
    pTempMsg->setSource(getIndex());
    pTempMsg->setDestination(getIndex());
    return pTempMsg;
}//generateMessage()

void OutNode01::handleMessage(cMessage * pHandleMsg) {
    utilityUNUSED(pHandleMsg);
    
    pMsg = generateMessage();
    EV << "sending message: " << pMsg->getName() << "\n";
    send(pMsg, "oport");
    pMsg = nullptr;
    scheduleAt(simTime()+1, pDriverMsg);
}// handleMessage(pHandleMsg)

/// --------------------------------------------------
/// InNode

InNode01::InNode01() {
    pMsg = nullptr;
}// constructor

InNode01::~InNode01() {
    if(pMsg)
        delete pMsg;
}// destructor

void InNode01::initialize() {
    pMsg = nullptr;
}//initialize()

void InNode01::finish() {
    
}//finish()

void InNode01::handleMessage(cMessage * pHandleMsg) {
    EV << "Receiving Message: " << pHandleMsg->getName() << "\n";
    InfoMessage01 * imsg = check_and_cast<InfoMessage01 *>(pHandleMsg);
    delete imsg;
}//handleMessage(pHandleMsg)

/// --------------------------------------------------
/// Router

Router01::Router01() {}

Router01::~Router01() {
}//destructor

void Router01::initialize() {}

void Router01::finish() {}

void Router01::forwardMessage(InfoMessage01 * pInfoMsg) {
    int ngates = gateSize("oport");
    int toGate = pInfoMsg->getDestination();
    if(toGate >= ngates) {
        throw std::out_of_range("Router::forwardMessage: "
                                "trying to send a message to a non-exist destination!");
    }//ifdef
    send(pInfoMsg, "oport", toGate);
    EV << "forward message to gate: " << toGate << "\n";
}//forwardMessage(pInfoMsg)

void Router01::handleMessage(cMessage * pHandleMsg) {
    InfoMessage01 * imsg = check_and_cast<InfoMessage01 *>(pHandleMsg);
    forwardMessage(imsg);
}//handleMessage(pHandleMsg)

