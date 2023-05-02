#include "../include/string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(string_view str) {
    vector<string_view> result;
    str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));

    while (!str.empty()) {
        size_t space = str.find(' ');
        result.push_back(space == str.npos ? str.substr(0) : str.substr(0, space));
        str.remove_prefix(result.back().size());
        str.remove_prefix(std::min(str.find_first_not_of(' '), str.size()));
    }
    return result;
}

