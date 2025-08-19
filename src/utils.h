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

#define ERROR_PRINT(x) std::cout << "\033[31m " << (x) << "\033[0m" << std::endl
#define WARNING_PRINT(x) std::cout << "\033[33m " << (x) << "\033[0m" << std::endl
#define INFO_PRINT(x) std::cout << "\033[32m " << (x) << "\033[0m" << std::endl
#define LOG_PRINT(x) std::cout << (x) << std::endl
#define PRINTF(a) (std::cout << "" << (#a) << " = " << (a) << "" << std::endl)
#define RELEASE(p) do{if (p != nullptr) delete (p); (p) = nullptr;}while(0)
#define RELEASES(p) do{if (p != nullptr) delete[] (p); (p) = nullptr;}while(0)
#define FREE(x) do{if (nullptr != (x)) {free((x)); (x) = nullptr;}}while(0)


#endif //UTILS_H
