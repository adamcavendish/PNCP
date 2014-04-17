#include "InNode.hpp"

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

InNode04::InNode04() :
    stat_timePassed(0),
    stat_packageReceived(0)
{}

InNode04::~InNode04() {
}// destructor

void InNode04::initialize() {
    std::string logPath = par("logPath");
    stat_logger.open(logPath);
}//initialize()

void InNode04::finish() {
    for(auto i : queue) {
        delete i;
    }//for
    queue.clear();
    stat_logger.close();
}//finish()

void InNode04::handleMessage(cMessage * pHandleMsg) {
    // edit statistics
    ++stat_packageReceived;

    InfoMessage04 * imsg = check_and_cast<InfoMessage04 *>(pHandleMsg);
    filterMessage(imsg);
}//handleMessage(pHandleMsg)

void InNode04::filterMessage(InfoMessage04 * pInfoMsg) {
    if(pInfoMsg->getDestination() == getIndex()) {
        useMessage(pInfoMsg);
    } else {
        if(pInfoMsg->getSource() != SOURCE_NODE::SROUTER) {
            addMessageToQueue(pInfoMsg);
        } else {
            if(pInfoMsg->getIsCoded() == true) {
                InfoMessage04 * imsg = decodePacket(pInfoMsg);
                useMessage(imsg);
            }//if
        }//if-else
    }//if-else
}//filterMessage(pInfoMsg)

void InNode04::useMessage(InfoMessage04 * pInfoMsg) {
    if(pInfoMsg->getDestination() != getIndex()) {
        EV << "InNode <" << getIndex() << " > -- THE MESSAGE IS NOT SENT TO ME!\n";
        utility::dump(pInfoMsg);
        throw std::runtime_error("THE MESSAGE IS NOT SENT TO ME!");
    }//if

    // if the message is sent to me
    EV << "InNode<" << getIndex() << "> -- Receiving Message\n";
    utility::dump(pInfoMsg);
    delete pInfoMsg;
}//useMessage(pInfoMsg)

void InNode04::addMessageToQueue(InfoMessage04 * pInfoMsg) {
    queue.push_front(pInfoMsg);
}//addMessageToQueue(pInfoMsg)

InfoMessage04 * InNode04::decodePacket(InfoMessage04 * pInfoMsg) {
    if(queue.empty())
        throw std::runtime_error("InNode queue is empty!");

    InfoMessage04 * prevMsg = queue.back();
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

    InfoMessage04 * decodedPacket = new InfoMessage04(tempMsgStr);
    utility::copy_str_n(tempMsgStr, prevMsgLength, decodedPacket);
    decodedPacket->setIsCoded(false);
    decodedPacket->setSource(SOURCE_NODE::SROUTER);
    decodedPacket->setDestination(getIndex());
    decodedPacket->setMsgLength(prevMsgLength);

    return decodedPacket;
}//decodePacket(pInfoMsg)

void InNode04::writeStatistics() {
    stat_logger.log() << stat_timePassed << ", " << stat_packageReceived
                      << std::endl;
    ++stat_timePassed;
}//writeStatistics()

