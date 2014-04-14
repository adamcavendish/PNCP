#include "Router.hpp"

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
#include "utility/utility.hpp"

Router04::Router04() :
    udist_codePacketProb(0, 1),
    pDriverMsg(nullptr)
{}

Router04::~Router04() {
    if(pDriverMsg)
        delete pDriverMsg;

    finish();
}//destructor

void Router04::initialize() {
    queueLength = par("queueLength");
    codePacketProb = par("codePacketProb");

    pDriverMsg = new cMessage("");
}//initialize()

void Router04::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;

    EV << "%%%%%%%%%%%%%%%%%%%% DROPING Packets: " << (queue[0].size() + queue[1].size())
       << " %%%%%%%%%%%%%%%%%%%%\n";
    for(int i = 0; i < 2; ++i) {
        for(auto j : queue[i]) {
            delete j;
        }//for
        queue[i].clear();
    }//for
}//finish()

void Router04::handleMessage(cMessage * pHandleMsg) {
    // if receive a general message
    if(pHandleMsg != pDriverMsg) {
        InfoMessage04 * imsg = check_and_cast<InfoMessage04 *>(pHandleMsg);
        addMessageToQueue(imsg);

        scheduleAt(simTime(), pDriverMsg);
        return;
    }//if-else

    // if receive a driver message
    // check whether to forward the message
    if(forwardPacketCondition() == true) {
        forwardOneMessageInQueue();
    } else {
        codeOrWaitMessageInQueue();
    }//if-else

    // if the queue is not empty, schedule a driver message
    if(queue[0].size() > 0 || queue[1].size() > 0)
        scheduleAt(simTime() + 1, pDriverMsg);
}//handleMessage(pHandleMsg)

void Router04::forwardMessage(InfoMessage04 * pInfoMsg) {
    int ngates = gateSize("oport");
    int toGate = pInfoMsg->getDestination();
    if(toGate >= ngates) {
        throw std::out_of_range("Router::forwardMessage: "
                                "trying to send a message to a non-exist destination!");
    }//ifdef

    pInfoMsg->setSource(SOURCE_NODE::SROUTER);
    send(pInfoMsg, "oport", toGate);

    EV << "********************forward message to gate: " << toGate << "********************\n";
}//forwardMessage(pInfoMsg)

void Router04::addMessageToQueue(InfoMessage04 * pInfoMsg) {
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

void Router04::broadcastCopyOfMessage(InfoMessage04 * pInfoMsg) {        
    EV << "********************Broadcast Message********************\n";
    utility::dump(pInfoMsg);

    int ngates = gateSize("oport");
    for(int i = 0; i < ngates; ++i) {
        utility::sendCopyOf(this, pInfoMsg, "oport", i);
    }//for
}//broadcastMessage(pInfoMsg)

bool Router04::codePacketCondition() {
    return (queue[0].size() > 0 && queue[1].size() > 0);
}//codePacketCondition()

bool Router04::forwardPacketCondition() {
    double prob = udist_codePacketProb(engine);
    double probCheck = 1 - pow(codePacketProb, queue[0].size() + queue[1].size());
    EV << "Router -- prob: " << prob << " Transmition Probabitlity: " << probCheck << "\n";
    return (prob <= probCheck);
}//forwardPacketCondition()

void Router04::forwardOneMessageInQueue() {
    int maxSizedIndex = utility::get_max_sized_queue(queue, queue + QUEUE_NUM);
    if(queue[maxSizedIndex].size() == 0) {
        waitForPacket();
        return;
    }//if

    InfoMessage04 * msg = queue[maxSizedIndex].back();

    EV << "********************Trying to forward packet********************\n";
    utility::dump(msg);

    forwardMessage(msg);

    queue[maxSizedIndex].pop_back();
}//forwardOneMessageInQueue()

void Router04::codeOrWaitMessageInQueue() {
    if(codePacketCondition() == true) {
        InfoMessage04 * codedMsg = codePacket();
        broadcastCopyOfMessage(codedMsg);
        delete codedMsg;
        return;
    } else {
        waitForPacket();
    }//if-else
}//codeOrWaitMessageInQueue()

InfoMessage04 * Router04::codePacket() {
    InfoMessage04 * msg0 = queue[0].back();
    InfoMessage04 * msg1 = queue[1].back();

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

    InfoMessage04 * codedPacket = new InfoMessage04("");
    utility::copy_str_n(tempMsg, msg0Length, codedPacket);
    codedPacket->setIsCoded(true);
    codedPacket->setSource(SOURCE_NODE::SROUTER);
    codedPacket->setDestination(DEST_NODE::DROUTER);
    codedPacket->setMsgLength(msg0Length);

    EV << "********************Coded Packet********************:\n";
    utility::dump(codedPacket);

    return codedPacket;
}//codePacket()

void Router04::waitForPacket() {
    EV << "Router -- wait for packet\n";
}//waitForPacket()
