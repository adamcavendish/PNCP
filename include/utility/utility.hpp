#pragma once
// STL
#include <vector>
#include <list>
#include <algorithm>

#define utilityUNUSED(x) (void)(x)

namespace utility {

std::vector<char>
xorPacket(const std::vector<char> & packet1, const std::vector<char> & packet2);

inline std::vector<char>
codePacket(const std::vector<char> & packet1, const std::vector<char> & packet2)
{ return xorPacket(packet1, packet2); }

inline std::vector<char>
decodePacket(const std::vector<char> & codedPacket, const std::vector<char> & packet)
{ return xorPacket(codedPacket, packet); }

}//namespace utility
