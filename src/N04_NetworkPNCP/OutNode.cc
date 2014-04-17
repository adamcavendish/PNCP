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
#include "logger/logger.hpp"

OutNode04::OutNode04() :
    engine(rd()),
    udist_generatePacketProb(0, 1),
    pDriverMsg(nullptr),
    pMsg(nullptr),
    stat_timePassed(0),
    stat_packageSent(0)
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
    pDriverMsg = new DriverMessage04("");
    pMsg = generateMessage();

    std::string logPath = par("logPath");
    stat_logger.open(logPath);
    WATCH(stat_timePassed);
    WATCH(stat_packageSent);

    scheduleAt(simTime(), pDriverMsg);
}// initialize()

void OutNode04::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;
    stat_logger.close();
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

    EV << "OutNode<" << getIndex() << "> -- generate message\n";
    utility::dump(pMsg);
    utility::sendCopyOf(this, pMsg, "oport");
    utility::sendCopyOf(this, pMsg, "connect");
    scheduleAt(simTime()+1, pDriverMsg);

    writeStatistics();
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

    // edit statistics
    ++stat_packageSent;

    return pTempMsg;
}//generateMessage()

void OutNode04::writeStatistics() {
    /**
     * format:
     * time-passed, package-sent, package-sending-percentage
     */
    // `ps` means package sent
    // `pct` means percentage
    double ps_pct = (stat_timePassed > 0) ? static_cast<double>(stat_packageSent)/stat_timePassed : 0;
    stat_logger.log() << stat_timePassed << ", " << stat_packageSent << ", " << ps_pct << std::endl;

    ++stat_timePassed;
}//writeStatistics()
