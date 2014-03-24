#include <utility/utility.hpp>
// STL
#include <cassert>
#include <vector>
#include <algorithm>

namespace utility {

std::vector<char>
xorPacket(const std::vector<char> & packet1, const std::vector<char> & packet2) {
    assert((packet1.size() == packet2.size()) && "packet1's size should match packet2's size");
    std::vector<char> ret(packet1.size());
    std::transform(packet1.begin(), packet1.end(), packet2.begin(), ret.begin(),
                   [](const char & a, const char & b) { return a xor b; });
    return ret;
}//xorPacket(packet1, packet2)

}//namespace utility
