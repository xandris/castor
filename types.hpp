#ifndef TYPES_HPP
#define TYPES_HPP

#include "config.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <boost/core/demangle.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::for_each;
using std::pair;
using std::shared_ptr;
using std::span;
using std::string;
using std::string_view;
using std::unique_ptr;
using std::vector;

using namespace std::literals::string_literals;
using namespace std::literals::chrono_literals;

using system_error = boost::system::system_error;
using err = boost::system::error_code;

#endif