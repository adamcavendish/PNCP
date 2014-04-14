#pragma once
// STL
#include <utility>
#include <vector>
#include <list>
#include <algorithm>
#include <random>
// omnetpp
#include <omnetpp.h>

#define utilityUNUSED(x) (void)(x)

enum SOURCE_NODE {
    SROUTER = -1,
    S0 = 0,
    S1 = 1
};//enum SOURCE_NODE

enum DEST_NODE {
    DROUTER = -1,
    D0 = 0,
    D1 = 1
};//enum DEST_NODE

namespace utility {

template <typename msgType>
void sendCopyOf(cSimpleModule * module, msgType * pMsg, const char * port) {
    msgType * copy = check_and_cast<msgType *>(pMsg->dup());
    module->send(copy, port);
}//sendCopyOf(module, pMsg, port)

template <typename msgType>
void sendCopyOf(cSimpleModule * module, msgType * pMsg, const char * port, const int portId) {
    msgType * copy = check_and_cast<msgType *>(pMsg->dup());
    module->send(copy, port, portId);
}//sendCopyOf(module, pMsg, port, portId)

/**
 *  @brief dump() is used to dump InfoMessage for debug or log usage
 */
template <typename T>
void dump(T * pMsg) {
    int pMsgLength = pMsg->getMsgLength();

    if(pMsg->getIsCoded()) {     
        EV << "  Message: This is a coded message.\n";
    } else {
        EV << "  Message: ";
        for(int i = 0; i < pMsgLength; ++i) {
            EV << pMsg->getRawMessage(i);
        }//for
        EV << "\n";
    }//if-else
    EV << "  binary format: [" << pMsg->getMsgLength() << "] ";
    for(int i = 0; i < pMsgLength; ++i) {
        EV << static_cast<unsigned int>(pMsg->getRawMessage(i)) << " ";
    }//for
    EV << "\n";
    EV << "  IsCoded: " << pMsg->getIsCoded()
       << "  S: " << pMsg->getSource()
       << "  D: " << pMsg->getDestination() << "\n";
}//dump(pMsg)

template <typename MsgType>
void copy_str_n(const char * str_beg, std::size_t n, MsgType * pMsg) {
    for(int i = 0; i < n; ++i)
        pMsg->setRawMessage(i, *(str_beg + i));
    pMsg->setRawMessage(n, '\0');
}//copy_str_n(str_beg, n, pMsg)

/**
 * Because c++11 do not allow template lambda so I just reimplement this for easy using.
 *
 * @brief extract the maximum queue's index
 * @return index of the maximum sized queue. if no queue in the range, return -1.
 */
template <typename ForwardIterator>
int
get_max_sized_queue(ForwardIterator beg, ForwardIterator end) {
    if(beg == end)
        return -1;

    int largest_index = 0;
    ForwardIterator largest = beg;
    for(int i = 0; beg != end; ++beg, ++i) {
        if(largest->size() < beg->size()) {
            largest = beg;
            largest_index = i;
        }//if
    }//while
    return largest_index;
}//get_max_sized_queue(beg, end)

}//namespace utility
