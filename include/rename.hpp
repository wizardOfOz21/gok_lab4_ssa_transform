#pragma once 

#include <string>
#include <sstream>

std::string name_iter(std::string name, int i) {
    std::ostringstream ss;
    ss << name << i;
    return ss.str();
}
