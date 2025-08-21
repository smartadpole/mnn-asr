//
// Created by smart on 2025/8/19.
//

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <dirent.h>


#define FUNC_NAME __FUNCTION__
const std::string MODULE_NAME = "SpeechReconstruction";
#define FILE (std::string(__FILE__).substr(std::string(__FILE__).find(std::string("src"))))
#define FILE_LINE (FILE + ":" + std::to_string(__LINE__))
#define MODULE_LINE ("[" + std::string(MODULE_NAME) + "] [" + FILE_LINE) + "] "
#define MODULE_LINE_FUNCTION (MODULE_LINE + "[" + std::string(FUNC_NAME) + "] ")

class LogStream {
public:
    enum LogLevel { ERROR, WARNING, INFO, LOG, DEBUG, TRACE };

    LogStream(LogLevel level) : level_(level) {}

    template<typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

    ~LogStream() {
        std::string message = stream_.str();
        switch(level_) {
        case ERROR:
            std::cout << "\033[31m" << "[ERROR] " << message << "\033[0m" << std::endl;
            break;
        case WARNING:
            std::cout << "\033[33m" << "[WARNING] " << message << "\033[0m" << std::endl;
            break;
        case INFO:
            std::cout << "\033[32m" << "[INFO] " << message << "\033[0m" << std::endl;
            break;
        case LOG:
            std::cout << "[LOG] " << message << std::endl;
            break;
        case DEBUG:
            std::cout << "\033[34m" << "[DEBUG] " << message << "\033[0m" << std::endl;
            break;
        case TRACE:
            std::cout << "\033[35m" << "[TRACE] " << message << "\033[0m" << std::endl;
            break;
        default:
            std::cout << message << std::endl;
            break;
        }
    }

private:
    LogLevel level_;
    std::stringstream stream_;
};

// log
#define LOG_ERROR LogStream(LogStream::ERROR)
#define LOG_WARNING LogStream(LogStream::WARNING)
#define LOG_INFO LogStream(LogStream::INFO)
#define LOG_DEBUG LogStream(LogStream::DEBUG)
#define LOG LogStream(LogStream::LOG)


// print
#define ERROR_PRINT(x) LOG_ERROR << MODULE_LINE_FUNCTION << (x)
#define WARNING_PRINT(x) LOG_WARNING << MODULE_LINE_FUNCTION << (x)
#define INFO_PRINT(x) LOG_INFO << MODULE_LINE_FUNCTION << (x)
#ifdef NDEBUG
#define DEBUG_PRINT(x)
#define TIMING_DEBUG(x)
#else
#define DEBUG_PRINT(x) LOG_DEBUG << MODULE_LINE_FUNCTION << (x)
#define TIMING_DEBUG(x) //LogStream(LogStream::TRACE) << MODULE_LINE_FUNCTION << (x)
#endif
#define LOG_PRINT(x) LOG << MODULE_LINE_FUNCTION << (x)

#define TIMING(x) LogStream(LogStream::TRACE) << MODULE_LINE_FUNCTION << (x)

#define PRINTF(a) (std::cout << "" << (#a) << " = " << (a) << "" << std::endl)
#define RELEASE(p) do{if (p != nullptr) delete (p); (p) = nullptr;}while(0)
#define RELEASES(p) do{if (p != nullptr) delete[] (p); (p) = nullptr;}while(0)
#define FREE(x) do{if (nullptr != (x)) {free((x)); (x) = nullptr;}}while(0)
#endif //UTILS_H
