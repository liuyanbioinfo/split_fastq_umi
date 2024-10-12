// include/Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <set>

int hamming_distance(const std::string& a, const std::string& b);
std::set<int> parse_allowed_positions(const std::string& p_str);

#endif // UTILS_H

