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
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N03_NetworkPNCP_m.h"
#include "utility/utility.hpp"

class OutNode03 : public cSimpleModule {
private:
    int counter;
    cMessage * pDriverMsg; // this message drives me to send out a message 'msg'
    InfoMessage03 * pMsg; // the real message send to other nodes
public:
    OutNode03();
    virtual ~OutNode03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    void sendCopyOf(cMessage * pMsg, const char * port);
    InfoMessage03 * generateMessage();
};

class InNode03 : public cSimpleModule {
private:
    InfoMessage03 * pMsg; // point to the message received
public:
    InNode03();
    virtual ~InNode03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
};

class Router03 : public cSimpleModule {
private:
    int queueLength;
    std::deque<InfoMessage03> queueForD1;
    std::deque<InfoMessage03> queueForD2;
public:
    Router03();
    virtual ~Router03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    void forwardMessage(InfoMessage03 * pInfoMsg);
};

Define_Module(OutNode03);
Define_Module(InNode03);
Define_Module(Router03);

/// --------------------------------------------------
/// implementation part

/// --------------------------------------------------
/// OutNode

OutNode03::OutNode03() {
    counter = 0;
    pDriverMsg = nullptr;
    pMsg = nullptr;
}// constructor

OutNode03::~OutNode03() {
    if(pDriverMsg)
        delete pDriverMsg;
    if(pMsg)
        delete pMsg;
}// destructor

void OutNode03::initialize() {
    counter = par("counter");
    pDriverMsg = new cMessage("");
    pMsg = generateMessage();
    scheduleAt(simTime(), pDriverMsg);
}// initialize()

void OutNode03::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;
}//finish()

InfoMessage03 * OutNode03::generateMessage() {
    const int msgStrSize = 64;
    char msgStr[msgStrSize];
    snprintf(msgStr, msgStrSize, "Msg from S%d to D%d", getIndex(), getIndex());

    InfoMessage03 * pTempMsg = new InfoMessage03(msgStr);
    pTempMsg->setSource(getIndex());
    pTempMsg->setDestination(getIndex());
    return pTempMsg;
}//generateMessage()

void OutNode03::handleMessage(cMessage * pHandleMsg) {
    utilityUNUSED(pHandleMsg);

    if(counter == 0)
        return;
    if(counter > 0)
        --counter;
    
    EV << "sending message: " << pMsg->getName() << "\n";
    sendCopyOf(pMsg, "oport");
    sendCopyOf(pMsg, "connect");
    scheduleAt(simTime()+1, pDriverMsg);
}// handleMessage(pHandleMsg)

void OutNode03::sendCopyOf(cMessage * pMsg, const char * port) {
    cMessage * copy = (cMessage *)pMsg->dup();
    send(copy, port);
}// sendCopyOf(pMsg, port)

/// --------------------------------------------------
/// InNode

InNode03::InNode03() {
    pMsg = nullptr;
}// constructor

InNode03::~InNode03() {
    if(pMsg)
        delete pMsg;
}// destructor

void InNode03::initialize() {
    pMsg = nullptr;
}//initialize()

void InNode03::finish() {
    
}//finish()

void InNode03::handleMessage(cMessage * pHandleMsg) {
    EV << "Receiving Message: " << pHandleMsg->getName() << "\n";
    InfoMessage03 * imsg = check_and_cast<InfoMessage03 *>(pHandleMsg);
    delete imsg;
}//handleMessage(pHandleMsg)

/// --------------------------------------------------
/// Router
Router03::Router03() {}

Router03::~Router03() {
}//destructor

void Router03::initialize() {}

void Router03::finish() {}

void Router03::forwardMessage(InfoMessage03 * pInfoMsg) {
    int ngates = gateSize("oport");
    int toGate = pInfoMsg->getDestination();
    if(toGate >= ngates) {
        throw std::out_of_range("Router::forwardMessage: "
                                "trying to send a message to a non-exist destination!");
    }//ifdef
    send(pInfoMsg, "oport", toGate);
    EV << "forward message to gate: " << toGate << "\n";
}//forwardMessage(pInfoMsg)

void Router03::handleMessage(cMessage * pHandleMsg) {
    InfoMessage03 * imsg = check_and_cast<InfoMessage03 *>(pHandleMsg);
    forwardMessage(imsg);
}//handleMessage(pHandleMsg)

