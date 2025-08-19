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
const std::string MODULE_NAME = "ASR";

class LogStream {
public:
    enum LogLevel { ERROR, WARNING, INFO, LOG };

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
            std::cout << "\033[31m " << message << "\033[0m" << std::endl;
            break;
        case WARNING:
            std::cout << "\033[33m " << message << "\033[0m" << std::endl;
            break;
        case INFO:
            std::cout << "\033[32m " << message << "\033[0m" << std::endl;
            break;
        case LOG:
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
#define LOG LogStream(LogStream::LOG)


// print
#define ERROR_PRINT(x) LOG_ERROR << "[" + std::string(MODULE_NAME) + "] [" << __FILE__ << ":" << __LINE__ << "] [" + std::string(FUNC_NAME) + "] " << (x)
#define WARNING_PRINT(x) LOG_WARNING << "[" + std::string(MODULE_NAME) + "] [" << __FILE__ << ":" << __LINE__ << "] [" + std::string(FUNC_NAME) + "] " << (x)
#define INFO_PRINT(x) LOG_INFO << "[" + std::string(MODULE_NAME) + "] [" << __FILE__ << ":" << __LINE__ << "] [" + std::string(FUNC_NAME) + "] " << (x)
#define LOG_PRINT(x) LOG << "[" + std::string(MODULE_NAME) + "] [" << __FILE__ << ":" << __LINE__ << "] [" + std::string(FUNC_NAME) + "] " << (x)

#define TIMMING(x) INFO_PRINT(x)

#define PRINTF(a) (std::cout << "" << (#a) << " = " << (a) << "" << std::endl)
#define RELEASE(p) do{if (p != nullptr) delete (p); (p) = nullptr;}while(0)
#define RELEASES(p) do{if (p != nullptr) delete[] (p); (p) = nullptr;}while(0)
#define FREE(x) do{if (nullptr != (x)) {free((x)); (x) = nullptr;}}while(0)
#endif //UTILS_H
