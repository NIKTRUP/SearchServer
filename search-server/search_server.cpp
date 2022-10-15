#include "search_server.h"
#include <algorithm>
#include <numeric>
#include <cmath>

using namespace std;

SearchServer::SearchServer(const string& stop_words){
    SetStopWords(stop_words);
}

SearchServer::SearchServer(const char* stop_words){
    SetStopWords(static_cast<string>(stop_words));
}

int SearchServer::GetDocumentCount() const{
    return documents_.size();
}

void SearchServer::AddDocument(int document_id, const string& document,
                 DocumentStatus status, const vector<int>& ratings) {
    if(document_id < 0 || documents_.count(document_id) != 0){
        throw invalid_argument("Invalid document_id"s);
    }

    const vector<string> words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_id_direct_order_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus filter_status) const {
   return FindTopDocuments(raw_query, [filter_status](int , DocumentStatus status, int ){
       return status == filter_status;
   });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
       vector<string> matched_documents;
       DocumentStatus status;
       const auto query = ParseQuery(raw_query);

       if(documents_.count(document_id)){
           status = documents_.at(document_id).status;
       }else{
           throw invalid_argument("Данный id не найден"s);
       }

       if(!query.plus_words.empty()){
           for(const auto& minus_word: query.minus_words){
               if(word_to_document_freqs_.count(minus_word)){
                   if(word_to_document_freqs_.at(minus_word).count(document_id)){
                       return {matched_documents, status};
                   }
               }
           }

           for(const auto& plus_word: query.plus_words){
               if(word_to_document_freqs_.count(plus_word)){
                   if(word_to_document_freqs_.at(plus_word).count(document_id)){
                       matched_documents.push_back(plus_word);
                   }
               }
           }
       }
       return tie(matched_documents, status);
   }

int SearchServer::GetDocumentId(int index) const { // index - порядковый номер
       if(index < 0 || index > static_cast<int>(documents_.size())){
           throw out_of_range("Документа с таким порядковым номером не найдено"s);
       }
       return document_id_direct_order_[index];
 }

bool SearchServer::IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

void SearchServer::SetStopWords(const string& text) {
    for (const string& word : SplitIntoWords(text)) {
        if(!IsValidWord(word)){
            throw invalid_argument("Стоп-слово не может состоять из спецсимволов"s);
        }
        stop_words_.insert(word);
    }
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if(!IsValidWord(word)){
            throw invalid_argument("Word "s + word + " is invalid"s);
        }

        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0, [](const auto& l, const auto& r){
        return l + r;
    });
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string word) const {
    if(!IsValidWord(word)){
       throw invalid_argument("Поиск не произведён из-за невалидного запроса"s);
    }

    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);

        if(word.empty() || word[0] == '-'){
            throw invalid_argument("Поиск не произведён из-за невалидного запроса"s);
        }
    }
    return {word, is_minus, IsStopWord(word)};
}


SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(documents_.size() * 1.0 / word_to_document_freqs_.at(word).size());
}

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const invalid_argument& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const string& query) {
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) {
            const int document_id = search_server.GetDocumentId(index);
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const invalid_argument& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}

