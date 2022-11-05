#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H
#include <map>
#include <set>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "document.h"
#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5;



class SearchServer {
public:

    using MapWordFreq = std::map<std::string, double>;

    explicit SearchServer(const std::string& stop_words);

    explicit SearchServer(const char* stop_words);

    template<typename StringCollection>
    explicit SearchServer(const StringCollection& collection_stop_words);

    int GetDocumentCount() const;

    void AddDocument(int document_id, const std::string& document,
                     DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const ;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus filter_status) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const ;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const ;

    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    std::set<int>::const_iterator begin();

    std::set<int>::const_iterator end();

    //int GetDocumentId(int index) const; // index - порядковый номер

    void RemoveDocument(int document_id);

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
        MapWordFreq word_freq;
    };

    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::set<int> document_id_direct_order_;
    std::map<int, DocumentData> documents_;

    static bool IsValidWord(const std::string& word) ;

    void SetStopWords(const std::string& text) ;

    bool IsStopWord(const std::string& word) const ;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const ;

    static int ComputeAverageRating(const std::vector<int>& ratings) ;

    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string word) const ;

    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const ;

    double ComputeWordInverseDocumentFreq(const std::string& word) const ;

    template<typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const ;
};



template<typename StringCollection>
SearchServer::SearchServer(const StringCollection& collection_stop_words){
    for(const auto& stop_word: collection_stop_words){
        if(!IsValidWord(stop_word)){
            throw std::invalid_argument("Стоп-слово не может состоять из спецсимволов");
        }

        if(stop_word.size() != 0 ){
            stop_words_.insert(stop_word);
        }
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentPredicate document_predicate) const {
   std::vector<Document> matched_documents;
   const Query query = ParseQuery(raw_query);
   matched_documents = FindAllDocuments(query, document_predicate);

   sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
           double eps = 1e-6;
           if (std::abs(lhs.relevance - rhs.relevance) < eps) {
               return lhs.rating > rhs.rating;
           }
           return lhs.relevance > rhs.relevance;
        });
   if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
       matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
   }
   return matched_documents;
}

template<typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status ,document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            {document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query);

//void MatchDocuments(const SearchServer& search_server, const std::string& query);

void AddDocument(SearchServer& search_server, int document_id, const std::string& document,
                 DocumentStatus status, const std::vector<int>& ratings);

#endif // SEARCHSERVER_H
