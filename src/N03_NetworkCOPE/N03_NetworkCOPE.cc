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
#include "build/N03_NetworkCOPE_m.h"
#include "utility/utility.hpp"

class OutNode03 : public cSimpleModule {
private:
    std::default_random_engine engine;
    std::uniform_real_distribution<double> udist;
    int counter;           // counter is the number of packets to send
    double sendProb;       // sendProb is the send probability of every timeslot
    cMessage * pDriverMsg; // this message drives me to send out a message 'msg'
    InfoMessage03 * pMsg;  // the real message send to other nodes
public:
    OutNode03();
    virtual ~OutNode03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief check whether OutNode is going to send out a message
    bool checkSendMessage();
    /// @brief generate a message for OutNode to send to InNode
    InfoMessage03 * generateMessage();
};

class InNode03 : public cSimpleModule {
private:
    std::deque<InfoMessage03 *> queue;
public:
    InNode03();
    virtual ~InNode03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief filter the message to see whether the message is sent to me.
    void filterMessage(InfoMessage03 * pInfoMsg);
    /// @brief after check the message is sent to me, 'useMessage()' 'uses' the message and delete it
    void useMessage(InfoMessage03 * pInfoMsg);
    /// @brief when the message is not sent to me, add it to the queue for decode
    void addMessageToQueue(InfoMessage03 * pInfoMsg);
    /// @brief decode the message. 'pInfoMsg' is the coded message
    InfoMessage03 * decodePacket(InfoMessage03 * pInfoMsg);
};

class Router03 : public cSimpleModule {
private:
    int queueLength;
    std::deque<InfoMessage03 *> queue[2];
public:
    Router03();
    virtual ~Router03();
protected:
    virtual void initialize() override;
    virtual void finish() override;
    virtual void handleMessage(cMessage * pHandleMsg) override;
    /// @brief forward the Message to the corresponding InNode
    void forwardMessage(InfoMessage03 * pInfoMsg);
    /// @brief add message to the corresponding queue
    void addMessageToQueue(InfoMessage03 * pInfoMsg);
    /// @brief broadcast the message to the 2 InNodes
    void broadcastCopyOfMessage(InfoMessage03 * pInfoMsg);
    /// @brief check whether the condition to code packet is met up
    bool codePacketCondition();
    /// @brief code 2 packets in the queues
    InfoMessage03 * codePacket();
};

Define_Module(OutNode03);
Define_Module(InNode03);
Define_Module(Router03);

/// --------------------------------------------------
/// implementation part

/// --------------------------------------------------
/// OutNode

OutNode03::OutNode03() {
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
    sendProb = par("sendProb");
    pDriverMsg = new cMessage("");
    pMsg = generateMessage();
    scheduleAt(simTime(), pDriverMsg);
}// initialize()

void OutNode03::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;
}//finish()

void OutNode03::handleMessage(cMessage * pHandleMsg) {
    utilityUNUSED(pHandleMsg);

    if(counter == 0)
        return;
    if(counter > 0)
        --counter;

    if(!checkSendMessage()) {
        scheduleAt(simTime()+1, pDriverMsg);
        return;
    }//if

    EV << "********************OutNode<" << getName() << "> -- sending message"
       << "********************\n";
    utility::dump(pMsg);
    utility::sendCopyOf(this, pMsg, "oport");
    utility::sendCopyOf(this, pMsg, "connect");
    scheduleAt(simTime()+1, pDriverMsg);
}// handleMessage(pHandleMsg)

bool OutNode03::checkSendMessage() {
    double prob = udist(engine);
    EV << "OutNode<" << getIndex() << "> -- prob: " << prob << " sendProb: " << sendProb << "\n";
    return (prob <= sendProb);
}//checkSendMessage()

InfoMessage03 * OutNode03::generateMessage() {
    const int msgStrSize = 64;
    char msgStr[msgStrSize];
    snprintf(msgStr, msgStrSize, "Msg from S%d to D%d", getIndex(), getIndex());

    InfoMessage03 * pTempMsg = new InfoMessage03(msgStr);

    int msgLength = strlen(msgStr);
    utility::copy_str_n(msgStr, msgLength, pTempMsg);
    pTempMsg->setRawMessage(msgLength, '\0');
    pTempMsg->setIsCoded(false);
    pTempMsg->setSource(getIndex());
    pTempMsg->setDestination(getIndex());
    pTempMsg->setMsgLength(msgLength);
    return pTempMsg;
}//generateMessage()

/// --------------------------------------------------
/// InNode

InNode03::InNode03() {
}// constructor

InNode03::~InNode03() {
}// destructor

void InNode03::initialize() {
}//initialize()

void InNode03::finish() {
    for(auto i : queue) {
        delete i;
    }//for
    queue.clear();
}//finish()

void InNode03::handleMessage(cMessage * pHandleMsg) {
    InfoMessage03 * imsg = check_and_cast<InfoMessage03 *>(pHandleMsg);
    filterMessage(imsg);
}//handleMessage(pHandleMsg)

void InNode03::filterMessage(InfoMessage03 * pInfoMsg) {
    if(pInfoMsg->getDestination() == getIndex()) {
        useMessage(pInfoMsg);
    } else {
        if(pInfoMsg->getSource() != SOURCE_NODE::SROUTER) {
            addMessageToQueue(pInfoMsg);
        } else {
            InfoMessage03 * imsg = decodePacket(pInfoMsg);
            useMessage(imsg);
        }//if-else
    }//if-else
}//filterMessage(pInfoMsg)

void InNode03::useMessage(InfoMessage03 * pInfoMsg) {
    if(pInfoMsg->getDestination() != getIndex()) {
        EV << "THE MESSAGE IS NOT SENT TO ME!\n";
        utility::dump(pInfoMsg);
        throw std::runtime_error("THE MESSAGE IS NOT SENT TO ME!");
    }//if

    // if the message is sent to me
    EV << "********************InNode<" << getIndex() << "> -- Receiving Message"
       << "********************\n";
    utility::dump(pInfoMsg);
    delete pInfoMsg;
}//useMessage(pInfoMsg)

void InNode03::addMessageToQueue(InfoMessage03 * pInfoMsg) {
    queue.push_front(pInfoMsg);
}//addMessageToQueue(pInfoMsg)

InfoMessage03 * InNode03::decodePacket(InfoMessage03 * pInfoMsg) {
    if(queue.empty())
        throw std::runtime_error("InNode queue is empty!");

    InfoMessage03 * prevMsg = queue.back();
    int prevMsgLength = prevMsg->getMsgLength();
    int pInfoMsgLength = pInfoMsg->getMsgLength();
    if(prevMsgLength != pInfoMsgLength) {
        throw std::runtime_error("2 messages to be decoded should have the same length");
    }//if

    char tempMsgStr[prevMsgLength + 1];
    for(auto i = 0; i < prevMsgLength; ++i) {
        tempMsgStr[i] = prevMsg->getRawMessage(i) xor pInfoMsg->getRawMessage(i);
    }//for
    tempMsgStr[prevMsgLength] = '\0';

    delete prevMsg;
    queue.pop_back();
    delete pInfoMsg;

    InfoMessage03 * decodedPacket = new InfoMessage03(tempMsgStr);
    utility::copy_str_n(tempMsgStr, prevMsgLength, decodedPacket);
    decodedPacket->setIsCoded(false);
    decodedPacket->setSource(SOURCE_NODE::SROUTER);
    decodedPacket->setDestination(getIndex());
    decodedPacket->setMsgLength(prevMsgLength);

    return decodedPacket;
}//decodePacket(pInfoMsg)

/// --------------------------------------------------
/// Router
Router03::Router03() {}

Router03::~Router03() {
    finish();
}//destructor

void Router03::initialize() {
    queueLength = par("queueLength");
}//initialize()

void Router03::finish() {
    EV << "%%%%%%%%%%%%%%%%%%%%DROPING Packets: " << (queue[0].size() + queue[1].size())
       << "%%%%%%%%%%%%%%%%%%%%\n";
    for(int i = 0; i < 2; ++i) {
        for(auto j : queue[i]) {
            delete j;
        }//for
        queue[i].clear();
    }//for
}//finish()

void Router03::handleMessage(cMessage * pHandleMsg) {
    InfoMessage03 * imsg = check_and_cast<InfoMessage03 *>(pHandleMsg);
    addMessageToQueue(imsg);
    
    if(codePacketCondition() == true) {
        InfoMessage03 * codedMsg = codePacket();
        broadcastCopyOfMessage(codedMsg);
        delete codedMsg;
    }//if
}//handleMessage(pHandleMsg)

void Router03::forwardMessage(InfoMessage03 * pInfoMsg) {
    int ngates = gateSize("oport");
    int toGate = pInfoMsg->getDestination();
    if(toGate >= ngates) {
        throw std::out_of_range("Router::forwardMessage: "
                                "trying to send a message to a non-exist destination!");
    }//ifdef

    pInfoMsg->setSource(SOURCE_NODE::SROUTER);
    send(pInfoMsg, "oport", toGate);

    EV << "forward message to gate: " << toGate << "\n";
}//forwardMessage(pInfoMsg)

void Router03::addMessageToQueue(InfoMessage03 * pInfoMsg) {
    EV << "********************Add Message to queue********************\n";
    utility::dump(pInfoMsg);

    if(static_cast<int>(queue[0].size() + queue[1].size()) >= queueLength) {
        EV << "queue[0].size: " << queue[0].size() << "\n";
        EV << "queue[1].size: " << queue[1].size() << "\n";
        EV << "queueLength: " << queueLength << "\n";
        throw std::out_of_range("Router::addMessageToQueue queue is full now!");
    }//if

    int dest = pInfoMsg->getDestination();
    if(dest > 1 || dest < 0) {
        throw std::out_of_range("Router::handleMessage: "
                                "trying to push to a non-exist queue");
    } else {
        queue[dest].push_front(pInfoMsg);
    }//if-else
}//addMessageToQueue(pInfoMsg)

void Router03::broadcastCopyOfMessage(InfoMessage03 * pInfoMsg) {        
    EV << "********************Broadcast Message********************\n";
    utility::dump(pInfoMsg);

    int ngates = gateSize("oport");
    for(int i = 0; i < ngates; ++i) {
        utility::sendCopyOf(this, pInfoMsg, "oport", i);
    }//for
}//broadcastMessage(pInfoMsg)

bool Router03::codePacketCondition() {
    return (queue[0].size() > 0 && queue[1].size() > 0);
}//codePacketCondition()

InfoMessage03 * Router03::codePacket() {
    InfoMessage03 * msg0 = queue[0].back();
    InfoMessage03 * msg1 = queue[1].back();

    EV << "********************Trying to code packet 'msg0'********************\n";
    utility::dump(msg0);
    EV << "********************Trying to code packet 'msg1'********************\n";
    utility::dump(msg1);

    int msg0Length = msg0->getMsgLength();
    int msg1Length = msg1->getMsgLength();
    if(msg0Length != msg1Length) {
        throw std::runtime_error("2 messages to be coded should have the same length");
    }//if

    // use 'xor' to code message, output to 'tempMsg'
    char tempMsg[msg0Length + 1];
    for(int i = 0; i < msg0Length; ++i) {
        tempMsg[i] = msg0->getRawMessage(i) xor msg1->getRawMessage(i);
    }//for
    tempMsg[msg0Length] = '\0';

    delete msg0;
    delete msg1;
    queue[0].pop_back();
    queue[1].pop_back();

    InfoMessage03 * codedPacket = new InfoMessage03("");
    utility::copy_str_n(tempMsg, msg0Length, codedPacket);
    codedPacket->setIsCoded(true);
    codedPacket->setSource(SOURCE_NODE::SROUTER);
    codedPacket->setDestination(DEST_NODE::DROUTER);
    codedPacket->setMsgLength(msg0Length);

    EV << "********************Coded Packet********************:\n";
    utility::dump(codedPacket);

    return codedPacket;
}//codePacket()

