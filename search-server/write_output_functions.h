#ifndef WRITE_OUTPUT_FUNCTIONS_H
#define WRITE_OUTPUT_FUNCTIONS_H
#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <set>
#include <string>

template<typename Key, typename Value>
std::ostream& operator<<(std::ostream& out, const std::pair<Key, Value>& container) {
    out << container.first << ": " << container.second;
    return out;
}

template <typename Container>
void Print(std::ostream& out, const Container& container) {
    bool is_first = true;
    for (const auto& element : container) {
        if (!is_first) {
            out << ", ";
        }
        is_first = false;
        out << element;
    }
}

template <typename Element>
std::ostream& operator<<(std::ostream& out, const std::vector<Element>& container) {
    out << "[";
    Print(out, container);
    out << "]";
    return out;
}

template<typename Key, typename Value>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value>& container) {
    out << "{";
    Print(out, container);
    out << "}";
    return out;
}

template <typename Element>
std::ostream& operator<<(std::ostream& out, const std::set<Element>& container) {
    out << "{";
    Print(out, container);
    out << "}";
    return out;
}


#endif // WRITE_OUTPUT_FUNCTIONS_H
