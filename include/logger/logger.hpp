#pragma once

// STL
#include <string>
#include <fstream>
#include <stdexcept>

namespace logger {

class Logger;

class Logger {
private: //members
    std::ofstream m_ofs;
public: // functions
    Logger();
    ~Logger();

    void open(const std::string & logPath) {
        m_ofs.open(logPath);
        if(!m_ofs.is_open()) {
            throw std::runtime_error("Cannot open logging file: '" + logPath + "'");
        }//if
    }//open(logPath)
    
    void close()
    { m_ofs.close(); }

    std::ofstream & log() {
        if(!m_ofs.is_open())
            throw std::runtime_error("Cannot open logging file!");
        return m_ofs;
    }//log()
};//class Logger

}//namespace logger
