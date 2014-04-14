#include "OutNode.hpp"
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

OutNode04::OutNode04() :
    udist_generatePacketProb(0, 1),
    pDriverMsg(nullptr),
    pMsg(nullptr)
{}

OutNode04::~OutNode04() {
    if(pDriverMsg)
        delete pDriverMsg;
    if(pMsg)
        delete pMsg;
    finish();
}// destructor

void OutNode04::initialize() {
    counter = par("counter");
    generatePacketProb = par("generatePacketProb");
    pDriverMsg = new cMessage("");
    pMsg = generateMessage();
    scheduleAt(simTime(), pDriverMsg);
}// initialize()

void OutNode04::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;
}//finish()

void OutNode04::handleMessage(cMessage * pHandleMsg) {
    utilityUNUSED(pHandleMsg);

    if(counter == 0)
        return;
    if(counter > 0)
        --counter;

    if(!checkSendMessage()) {
        scheduleAt(simTime()+1, pDriverMsg);
        return;
    }//if

    EV << "********************OutNode<" << getIndex() << "> -- sending message"
       << "********************\n";
    utility::dump(pMsg);
    utility::sendCopyOf(this, pMsg, "oport");
    utility::sendCopyOf(this, pMsg, "connect");
    scheduleAt(simTime()+1, pDriverMsg);
}// handleMessage(pHandleMsg)

bool OutNode04::checkSendMessage() {
    double prob = udist_generatePacketProb(engine);
    EV << "OutNode<" << getIndex() << "> -- prob: " << prob << " generatePacketProb: " << generatePacketProb << "\n";
    return (prob <= generatePacketProb);
}//checkSendMessage()

InfoMessage04 * OutNode04::generateMessage() {
    const int msgStrSize = 64;
    char msgStr[msgStrSize];
    snprintf(msgStr, msgStrSize, "Msg from S%d to D%d", getIndex(), getIndex());

    InfoMessage04 * pTempMsg = new InfoMessage04(msgStr);

    int msgLength = strlen(msgStr);
    utility::copy_str_n(msgStr, msgLength, pTempMsg);
    pTempMsg->setRawMessage(msgLength, '\0');
    pTempMsg->setIsCoded(false);
    pTempMsg->setSource(getIndex());
    pTempMsg->setDestination(getIndex());
    pTempMsg->setMsgLength(msgLength);
    return pTempMsg;
}//generateMessage()
