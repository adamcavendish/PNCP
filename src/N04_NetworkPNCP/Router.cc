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
// boost
#include <boost/scope_exit.hpp>
// omnetpp
#include <omnetpp.h>
// current project
#include "build/N04_NetworkPNCP_m.h"
#include "utility/utility.hpp"
#include "logger/logger.hpp"

Router04::Router04() :
    engine(rd()),
    udist_forwardPacketProb(0, 1),
    pDriverMsg(nullptr),
    stat_logger(),
    stat_timePassed(0),
    stat_packageFrom{0, 0},
    stat_packageForwardNumber{0, 0},
    stat_packageCodeNumber(0),
    stat_packageLostNumber{0, 0},
    stat_waitTime(0),
    stat_avg_queue_sz{0.0, 0.0},
    stat_avg_queue_all_sz(0)
{}

Router04::~Router04() {
    if(pDriverMsg)
        delete pDriverMsg;

    finish();
}//destructor

void Router04::initialize() {
    std::string logPath = par("logPath");
    queueLength = par("queueLength");
    alphaProb = par("alphaProb");
    betaProb = par("betaProb");
    stat_logger.open(logPath);

    pDriverMsg = new DriverMessage04("");
    scheduleAt(simTime(), pDriverMsg);

    // statistics
    WATCH(stat_timePassed);
    WATCH(stat_packageFrom[0]);
    WATCH(stat_packageFrom[1]);
    WATCH(stat_packageForwardNumber[0]);
    WATCH(stat_packageForwardNumber[1]);
    WATCH(stat_packageCodeNumber);
    WATCH(stat_packageLostNumber[0]);
    WATCH(stat_packageLostNumber[1]);
    WATCH(stat_waitTime);
    WATCH(stat_avg_queue_sz[0]);
    WATCH(stat_avg_queue_sz[1]);
    WATCH(stat_avg_queue_all_sz);
}//initialize()

void Router04::finish() {
    cancelAndDelete(pDriverMsg);
    pDriverMsg = nullptr;
    stat_logger.close();

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
    auto driveMsgExit = [this] {
        // edit statistics
        writeStatistics();
    };

    // if receive a general message
    if(utility::checkType<DriverMessage04 *>(pHandleMsg) == false) {
        InfoMessage04 * imsg = check_and_cast<InfoMessage04 *>(pHandleMsg);
        addMessageToQueue(imsg);
        return;
    }//if

    // if receive a driver message
    scheduleAt(simTime() + 1, pDriverMsg);

    if(codeMessageInQueueOnConditionTrue() == true) {
        driveMsgExit();
        return;
    }//if

    // if the code condition is not met up
    // check whether to forward the message
    if(forwardPacketCondition() == true) {
        forwardOneMessageInQueue();
    } else {
        waitForPacket();
    }//if-else

    driveMsgExit();
}//handleMessage(pHandleMsg)

void Router04::forwardMessage(InfoMessage04 * pInfoMsg) {
    // edit statistics
    ++stat_packageForwardNumber[pInfoMsg->getSource()];

    int ngates = gateSize("oport");
    int toGate = pInfoMsg->getDestination();
    if(toGate >= ngates) {
        throw std::out_of_range("Router::forwardMessage: "
                                "trying to send a message to a non-exist destination!");
    }//ifdef

    pInfoMsg->setSource(SOURCE_NODE::SROUTER);
    send(pInfoMsg, "oport", toGate);

    EV << "Router < > -- forward message to gate: " << toGate << "\n";
    utility::dump(pInfoMsg);
}//forwardMessage(pInfoMsg)

void Router04::addMessageToQueue(InfoMessage04 * pInfoMsg) {
    int source = pInfoMsg->getSource();
    int dest = pInfoMsg->getDestination();
    // edit statistics
    ++stat_packageFrom[source];
    
    EV << "Router < > -- Add Message to queue\n";
    utility::dump(pInfoMsg);

    if(static_cast<int>(queue[0].size() + queue[1].size()) >= queueLength) {
        char buf[1024];
        snprintf(buf, 1024, "QUEUE IS FULL: queue[0]: %lu queue[1]: %lu", queue[0].size(), queue[1].size());

        EV << "Router < > -- " << buf <<  "\n";
        
        // display message
        bubble(buf);
        // edit statistics
        ++stat_packageLostNumber[source];
        
        delete pInfoMsg;
        return;
    }//if

    if(dest > 1 || dest < 0) {
        throw std::out_of_range("Router::handleMessage: "
                                "trying to push to a non-exist queue");
    } else {
        queue[dest].push_front(pInfoMsg);
    }//if-else

    EV << "Router < > -- queue[0]: " << queue[0].size() << " queue[1]: " << queue[1].size() << "\n";
}//addMessageToQueue(pInfoMsg)

void Router04::broadcastCopyOfMessage(InfoMessage04 * pInfoMsg) {        
    EV << "Router < > -- Broadcast Message\n";
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
    double codePacketProb =
        [&] { return (queue[0].size() > 0) ? alphaProb : betaProb; }();
    double prob = udist_forwardPacketProb(engine);
    double probCheck = 1 - pow(codePacketProb, queue[0].size() + queue[1].size());
    EV << "Router < > -- prob: " << prob << " Transmition Probabitlity: " << probCheck << "\n";
    return (prob <= probCheck);
}//forwardPacketCondition()

void Router04::forwardOneMessageInQueue() {
    int maxSizedIndex = utility::get_max_sized_queue(queue, queue + QUEUE_NUM);
    if(queue[maxSizedIndex].size() == 0) {
        waitForPacket();
        return;
    }//if

    InfoMessage04 * msg = queue[maxSizedIndex].back();
    forwardMessage(msg);

    queue[maxSizedIndex].pop_back();
}//forwardOneMessageInQueue()

bool Router04::codeMessageInQueueOnConditionTrue() {
    if(codePacketCondition() == true) {
        InfoMessage04 * codedMsg = codePacket();
        broadcastCopyOfMessage(codedMsg);
        delete codedMsg;
        return true;
    }//if
    return false;
}//codeOrWaitMessageInQueue()

InfoMessage04 * Router04::codePacket() {
    InfoMessage04 * msg0 = queue[0].back();
    InfoMessage04 * msg1 = queue[1].back();

    EV << "Router < > -- code packet 'msg0'\n";
    utility::dump(msg0);
    EV << "Router < > -- code packet 'msg1'\n";
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

    EV << "Router < > -- coded packet 'coded'\n";
    utility::dump(codedPacket);

    // edit statistics
    ++stat_packageCodeNumber;

    return codedPacket;
}//codePacket()

void Router04::waitForPacket() {
    EV << "Router < > -- wait for packet\n";
    // edit statistics
    ++stat_waitTime;
}//waitForPacket()

void Router04::writeStatistics() {
    /**
     * format:
     * (00) time-passed, \
     * (01) queue1-length, queue2-length, queue-all-length, \
     * (04) average-queue1-length, average-queue2-length, average-queue-all-length, \
     * (07) packet-from1-number, packet-from2-number, packet-from-all-number, \
     * (10) packet-forward1-number, packet-forward2-number, packet-forward-all-number, \
     * (13) packet-forward1-in-percentage, packet-forward2-in-percentage, packet-forward-all-in-percentage, \
     * (16) packet-code-number, \
     * (17) packet-code-in-percentage, \
     * (18) packet-loss1-number, packet-loss2-number, packet-loss-all-number, \
     * (21) packet-loss1-in-percentage, packet-loss2-in-percentage, packet-loss-all-in-percentage, \
     * (24) wait-time-number, \
     * (25) wait-time-in-percentage
     */

    // `qs` means: queue size
    std::size_t qs0 = queue[0].size();
    std::size_t qs1 = queue[1].size();
    std::size_t qsall = qs0 + qs1;
    // calc average
    double qs0_avg = static_cast<double>(stat_avg_queue_sz[0] * stat_timePassed + qs0) / (stat_timePassed + 1);
    double qs1_avg = static_cast<double>(stat_avg_queue_sz[1] * stat_timePassed + qs1) / (stat_timePassed + 1);
    double qsall_avg = static_cast<double>(stat_avg_queue_all_sz * stat_timePassed + qsall) / (stat_timePassed + 1);
    // `p` means: package
    std::size_t p0 = stat_packageFrom[0];
    std::size_t p1 = stat_packageFrom[1];
    std::size_t pall = p0 + p1;
    // `pf` means: package forwarded
    std::size_t pf0 = stat_packageForwardNumber[0];
    std::size_t pf1 = stat_packageForwardNumber[1];
    std::size_t pfall = pf0 + pf1;
    // `pct` means: percentage
    double pf0_pct = (p0 > 0) ? static_cast<double>(pf0) / p0 : 0;
    double pf1_pct = (p1 > 0) ? static_cast<double>(pf1) / p1 : 0;
    double pfall_pct = (pall > 0) ? static_cast<double>(pfall) / pall : 0;
    // `'pl` means: package lost
    std::size_t pl0 = stat_packageLostNumber[0];
    std::size_t pl1 = stat_packageLostNumber[1];
    std::size_t plall = pl0 + pl1;
    double pl0_pct = (p0 > 0) ? static_cast<double>(pl0) / p0 : 0;
    double pl1_pct = (p1 > 0) ? static_cast<double>(pl1) / p1 : 0;
    double plall_pct = (pall > 0) ? static_cast<double>(plall) / pall : 0;
    double code_pct = (pall > 0) ? static_cast<double>(stat_packageCodeNumber)/pall : 0;
    double wait_pct = (stat_timePassed > 0) ? static_cast<double>(stat_waitTime)/stat_timePassed : 0;
    stat_logger.log() << stat_timePassed << ", "
                      << qs0 << ", " << qs1 << ", " << qsall << ", "
                      << qs0_avg << ", " << qs1_avg << ", " << qsall_avg << ", "
                      << p0 << ", " << p1 << ", " << pall << ", "
                      << pf0 << ", " << pf1 << ", " << pfall << ", "
                      << pf0_pct << ", " << pf1_pct << ", " << pfall_pct << ", "
                      << stat_packageCodeNumber << ", "
                      << code_pct << ", "
                      << pl0 << ", " << pl1 << ", " << plall << ", "
                      << pl0_pct << ", " << pl1_pct << ", " << plall_pct << ", "
                      << stat_waitTime << ", "
                      << wait_pct
                      << std::endl;
    // write back
    stat_timePassed += 1;
    stat_avg_queue_sz[0] = qs0_avg;
    stat_avg_queue_sz[1] = qs1_avg;
    stat_avg_queue_all_sz = qsall_avg;
}//writeStatistics()

