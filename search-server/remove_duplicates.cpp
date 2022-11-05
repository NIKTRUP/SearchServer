#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::set<std::set<std::string>> existing_docs;
    std::vector<int> found_duplicates;

    for (int document_id : search_server) {
        auto& freqs = search_server.GetWordFrequencies(document_id);
        std::set<std::string> words;

        std::transform(freqs.begin(), freqs.end(), std::inserter(words, words.begin()),
            [](auto p) { return p.first; });

        if (existing_docs.count(words) > 0) {
            std::cout << "Found duplicate document id " << document_id << std::endl;
            found_duplicates.push_back(document_id);
        }
        else {
            existing_docs.insert(words);
        }
    }

    for (int id : found_duplicates) {
        search_server.RemoveDocument(id);
    }
}
