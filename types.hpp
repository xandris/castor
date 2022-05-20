#ifndef TYPES_HPP
#define TYPES_HPP

//#include "config.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <vector>

#include <boost/system/error_code.hpp>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::for_each;
using std::pair;
using std::span;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

using err = boost::system::error_code;

#endif