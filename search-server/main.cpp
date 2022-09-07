#define _GLIBCXX_DEBUG 1
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <cassert>
#include <vector>
#include <numeric>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

template <typename Function>
void RunTestImpl(Function function,const string& func) {
    function();
    cerr << func << " OK"s << endl;
}

#define RUN_TEST(func)  RunTestImpl( (func) , #func )

template<typename Key, typename Value>
ostream& operator<<(ostream& out, const pair<Key, Value>& container) {
    out << container.first << ": " << container.second;
    return out;
}

template <typename Container>
void Print(ostream& out, const Container& container) {
    bool is_first = true;
    for (const auto& element : container) {
        if (!is_first) {
            out << ", "s;
        }
        is_first = false;
        out << element;
    }
}

template <typename Element>
ostream& operator<<(ostream& out, const vector<Element>& container) {
    out << "["s;
    Print(out, container);
    out << "]"s;
    return out;
}

template<typename Key, typename Value>
ostream& operator<<(ostream& out, const map<Key, Value>& container) {
    out << "{"s;
    Print(out, container);
    out << "}"s;
    return out;
}

template <typename Element>
ostream& operator<<(ostream& out, const set<Element>& container) {
    out << "{"s;
    Print(out, container);
    out << "}"s;
    return out;
}


template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "Assert("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

vector<int> TakeEvens(const vector<int>& numbers) {
    vector<int> evens;
    for (int x : numbers) {
        if (x % 2 == 0) {
            evens.push_back(x);
        }
    }
    return evens;
}

map<string, int> TakeAdults(const map<string, int>& people) {
    map<string, int> adults;
    for (const auto& [name, age] : people) {
        if (age >= 18) {
            adults[name] = age;
        }
    }
    return adults;
}

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }
    int i = 2;
    while (i * i <= n) {
        if (n % i == 0) {
            return false;
        }
        ++i;
    }
    return true;
}

set<int> TakePrimes(const set<int>& numbers) {
    set<int> primes;
    for (int number : numbers) {
        if (IsPrime(number)) {
            primes.insert(number);
        }
    }
    return primes;
}





string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const{
        const Query query = ParseQuery(raw_query);
        vector<string> matched_documents;

        DocumentStatus status;
        if(documents_.count(document_id)){
            status = documents_.at(document_id).status;
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

        return {matched_documents, status};
    }

    int GetDocumentCount() const{
        return documents_.size();
    }

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }

    template<typename Filter>
    vector<Document> FindTopDocuments(const string& raw_query, Filter filter) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, filter);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                double eps = 1e-6;
                if (abs(lhs.relevance - rhs.relevance) < eps) {
                    return lhs.rating > rhs.rating;
                }
                return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus filter_status = DocumentStatus::ACTUAL) const{
        return FindTopDocuments(raw_query, [filter_status](int document_id, DocumentStatus status, int rating){
            return status == filter_status;
        });
    }

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0, [](const auto& l, const auto& r){
            return l + r;
        });
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
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

    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(documents_.size() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename Filter>
    vector<Document> FindAllDocuments(const Query& query, Filter filter) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (filter(document_id, document_data.status ,document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};


void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}

// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size() , 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id , doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestAddDocument(){

    {
        SearchServer server;
        server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
        server.AddDocument(2, "а"s, DocumentStatus::BANNED, {5,3,4,1});
        server.AddDocument(3, "б"s, DocumentStatus::IRRELEVANT, {5,3,4,1});
        server.AddDocument(4, "с"s, DocumentStatus::REMOVED, {5,3,4,1});
        auto doc = server.FindTopDocuments("пропала кошка"s);
        ASSERT_EQUAL_HINT(doc.size() , 1,
                          "Проверяет на добавление документов с разными статусами"s);
    }

    {
    SearchServer server;
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc = server.FindTopDocuments("пропала кошка"s);
    ASSERT_HINT(doc[0].relevance == 0 &&
           doc[0].id == 1 &&
           doc[0].rating == 13/4 ,
           "Проверяет на добавление одного документа и вычисление его рейтинга"s);
    }

    {
    SearchServer server;
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc = server.FindTopDocuments("кошка пропала"s);
    ASSERT_HINT(doc.size() == 1 &&
           doc[0].relevance == 0 &&
           doc[0].id == 1 &&
           doc[0].rating == 13/4 ,
            "Проверяет, что результат не зависит от перемены мест слов"s);
    }

    {
    SearchServer server;
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "пропала собака"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "потерялась лисица"s, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc = server.FindTopDocuments("лисица -пропала"s);
    ASSERT_EQUAL_HINT(doc.size() , 1,
                      "Проверяет на исключение документа из списка выдачи"s);
    }

    {
    SearchServer server;
    server.AddDocument(3, "кошка игривая пропала"s, DocumentStatus::ACTUAL, {4,4,4,4});
    server.AddDocument(2, "собака злая"s, DocumentStatus::BANNED, {4,4,4,4});
    server.AddDocument(1, "попугай крикливый нашёлся"s, DocumentStatus::REMOVED, {5,3,4,1});
    auto doc = server.FindTopDocuments("кошка злая потерялась"s);
    ASSERT_HINT(abs( doc[0].relevance - 0.3662040) < 1e-5, "Проверяет счёт релевантности");
    ASSERT_EQUAL_HINT(doc[0].rating, 4, "Проверяет счёт рейтинга");
    ASSERT_HINT(doc.size() == 1 && doc[0].id == 3, "Проверяет на работу фильтра"s);
    }

    {
    SearchServer server;
    server.AddDocument(1, "кошка игривая пропала"s, DocumentStatus::BANNED, {1,2,3,4});
    server.AddDocument(1, "кошка злая потерялась"s, DocumentStatus::BANNED, {4,4,4,4});
    auto doc = server.FindTopDocuments("кошка злая потерялась"s, DocumentStatus::BANNED);
    ASSERT_HINT(doc.size() == 1 &&
           doc[0].rating == 2,
           "Проверяет на добавление документов с одним и тем же индексом"s );
    }

    {
    SearchServer server;
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(4, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(5, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(6, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(7, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc = server.FindTopDocuments("пропала кошка"s);
    ASSERT_EQUAL_HINT(doc.size() , 5,
                      "Проверяет на огранчение по количеству в топе"s);
    }

}

void TestSetStopWords(){
    {
    SearchServer server;

    server.SetStopWords("по и или на с до за не"s);

    server.AddDocument(1, "машина с беспилотным управлением и печкой"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(2, "автомобиль пропал"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});

    auto doc = server.FindTopDocuments("машина"s);
    ASSERT( abs(doc[0].relevance - 0.17328679) < 1e-6);
    }
}

void TestMatchDocument(){
    {
    SearchServer server;

    server.AddDocument(1, "машина беспилотная"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(2, "автомобиль пропал"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(3, "машина пропала белая"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});

    auto doc1 = server.MatchDocument("машина -беспилотная"s, 1);
    ASSERT_EQUAL(get<0>(doc1).size() , 0);

    auto doc2 = server.MatchDocument("машина -беспилотная"s, 2);
    ASSERT_EQUAL(get<0>(doc2).size() , 0);

    auto doc3 = server.MatchDocument("машина -беспилотная пропала"s, 3);
    ASSERT_EQUAL(get<0>(doc3).size() , 2);
    }
}

void TestSortRelevance(){
    {
        SearchServer server;
        server.AddDocument(1, "робот утилизатор"s, DocumentStatus::ACTUAL, {5,5,5,5,5});
        server.AddDocument(2, "беспилотный самолёт"s, DocumentStatus::ACTUAL, {4,4,4,4});
        server.AddDocument(3, "новая посудомойка"s, DocumentStatus::ACTUAL, {3,3,3});
        server.AddDocument(4, "робот пылесос"s, DocumentStatus::ACTUAL, {2,2});

        auto doc1 = server.FindTopDocuments("робот утилизатор беспилотный"s);
        ASSERT( doc1.size() == 3 &&
                doc1[0].id == 1 &&
                doc1[1].id == 2 &&
                doc1[2].id == 4);

        server.AddDocument(5, "робот робот"s, DocumentStatus::ACTUAL, {1});
        auto doc2 = server.FindTopDocuments("робот утилизатор беспилотный"s);
        ASSERT( doc2.size() == 4 &&
                doc2[0].id == 1 &&
                doc2[1].id == 2 &&
                doc2[2].id == 5 &&
                doc2[3].id == 4);

        auto doc3 = server.FindTopDocuments("робот утилизатор беспилотный"s, [](int document_id, DocumentStatus status, int rating){
            return rating > 3;
        });
        ASSERT_EQUAL(doc3.size() , 2);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestSetStopWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortRelevance);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
#ifdef _GLIBCXX_DEBUG
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cerr << "Search server testing finished"s << endl;
#endif
}
