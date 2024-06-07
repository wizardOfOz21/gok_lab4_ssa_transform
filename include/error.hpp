#pragma once
#include <exception>
#include "utils.hpp"

class DeclareExeption : public std::exception
{
};

class CodegenExeption : public std::exception
{
};
