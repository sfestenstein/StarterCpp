/**
 * @file AsyncQueue.cpp
 * @brief AsyncQueue is a header-only template, this file is for explicit instantiations
 *
 * Since AsyncQueue is a template class, most of its implementation is in the header.
 * This file can be used for explicit template instantiations if needed to reduce
 * compile times for commonly used types.
 */

#include "utils/AsyncQueue.hpp"

#include <string>

namespace utils
{

// Explicit instantiations for common types (optional, for faster compilation)
// Uncomment these if you want to use these specific instantiations across
// multiple translation units:

// template class AsyncQueue<int>;
// template class AsyncQueue<std::string>;

} // namespace utils
