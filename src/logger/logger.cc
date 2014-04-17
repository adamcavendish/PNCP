#include "logger/logger.hpp"
// STL
#include <stdexcept>

namespace logger {
    Logger::Logger() :
        m_ofs()
    {}//constructor

    Logger::~Logger() {
        m_ofs.close();
    }//destructor
}//namespace logger
