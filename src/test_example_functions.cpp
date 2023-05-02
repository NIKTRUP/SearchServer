#include "../include/test_example_functions.h"

using namespace std;

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id, const vector<string_view>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string_view word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

void AddDocument(SearchServer& search_server, int document_id, string_view document, DocumentStatus status,
                 const vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const invalid_argument& e) {
        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, string_view raw_query) {
    LOG_DURATION_STREAM("Время операции"s, cout);
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const invalid_argument& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, string_view query) {
    LOG_DURATION_STREAM("Время операции"s, cout);
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;

        for (int document_id : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const invalid_argument& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}

void RemoveDuplicates(SearchServer& search_server) {
    std::set<std::set<std::string_view>> existing_docs;
    std::vector<int> found_duplicates;

    for (int document_id : search_server) {
        auto& freqs = search_server.GetWordFrequencies(document_id);
        std::set<std::string_view> words;

        std::transform(freqs.begin(), freqs.end(), std::inserter(words, words.begin()),
        [](auto p) {
            return p.first;
        });

        if (existing_docs.count(words) > 0) {
            cout << "Found duplicate document id " << document_id << endl;
            found_duplicates.push_back(document_id);
        } else {
            existing_docs.insert(words);
        }
    }

    for (int id : found_duplicates) {
        search_server.RemoveDocument(id);
    }
}


// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = {1, 2, 3};
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        auto res = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(res.size() , 1);
        const Document& doc0 = res[0];
        ASSERT_EQUAL(doc0.id , doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.FindTopDocuments("in"s).size(), 0);
    }
}

void TestAddDocument(){
    {
        SearchServer server(""s);
        auto doc1 = "пропала кошка"s;
        server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {5,3,4,1});
        auto doc2 = "а"s;
        server.AddDocument(2, doc2, DocumentStatus::BANNED, {5,3,4,1});
        auto doc3 = "б"s;
        server.AddDocument(3, doc3, DocumentStatus::IRRELEVANT, {5,3,4,1});
        auto doc4 = "с"s;
        server.AddDocument(4, doc4, DocumentStatus::REMOVED, {5,3,4,1});
        auto query = "пропала кошка"s;
        auto result = server.FindTopDocuments(query);
        //  Проверяет на добавление документов с разными статусами
        ASSERT_EQUAL(result.size() , 1);
    }

    {
    SearchServer server(""s);
    auto doc = "пропала кошка"s;
    server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc2 = "пропала кошка"s;
    auto result = server.FindTopDocuments(doc2);
    // Проверяет на добавление одного документа и вычисление его рейтинга
    ASSERT(result[0].relevance == 0 &&
           result[0].id == 1 &&
           result[0].rating == 13/4);
    }

    {
    SearchServer server(""s);
    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(0, doc, DocumentStatus::ACTUAL, {5,3,4,1});
    }  catch (...) {
        // Проверяет на добавление id == 0
        ASSERT(false);
    }

    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(-1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        // Проверяет на добавление отрицательного id
        ASSERT(true);
    }

    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(-2, doc , DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        // Проверяет на добавление отрицательного id
        ASSERT(true);
        }
    }

    {
    SearchServer server(""s);
    auto doc0 = "пропала кошка"s;
    server.AddDocument(0, doc0, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc01 = "пропала собака"s;
    server.AddDocument(1, doc01, DocumentStatus::ACTUAL, {5,3,4,1});

    try {
        auto doc1 ="пропала ящерица"s;
        server.AddDocument(0, doc1, DocumentStatus::ACTUAL, {5,3,4,1});
        auto doc2 =  "пропал попугай"s;
        server.AddDocument(1, doc2, DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        auto doc3 =  "пропала кошка"s;
        auto result = server.FindTopDocuments(doc3);
        // Проверяет на добавление документа с существующим id
        ASSERT_EQUAL(result.size(), 2);
        }
    }



    {
        SearchServer server(""s);
        try {
            server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2});
            ASSERT(false);
        }  catch (...) {
            // Проверяем на добавление документа со спецсимволами
            ASSERT(true);
        }

    }

    {
    SearchServer server(""s);
    auto doc = "пропала кошка"s;
    (void) server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});


    auto result = server.FindTopDocuments("кошка пропала"s);
    // Проверяет, что результат не зависит от перемены мест слов
    ASSERT(result.size() == 1 &&
           result[0].relevance == 0 &&
           result[0].id == 1 &&
           result[0].rating == 13/4 );
    }

    {
    SearchServer server(""s);
    auto doc1 = "пропала кошка"s;
    server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc2 = "пропала собака"s;
    server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc3 = "потерялась лисица"s;
    server.AddDocument(3, doc3, DocumentStatus::ACTUAL, {5,3,4,1});

    auto result = server.FindTopDocuments("лисица -пропала"s);
    //Проверяет на исключение документа из списка выдачи
    ASSERT_EQUAL(result.size() , 1);
    }

    {
    SearchServer server(""s);
    server.AddDocument(1, "лисица пропала"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "собака пропала"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "потерялась лисица"s, DocumentStatus::ACTUAL, {5,3,4,1});
    try {
        auto vec = server.FindTopDocuments("лисица --пропала"s);
        ASSERT(false);
    }  catch (...) {
        // Проверяет на обработку --
        ASSERT(true);
    }


    }

    {
    SearchServer server(""s);
    auto doc1 = "кошка игривая пропала"s;
    server.AddDocument(3, doc1, DocumentStatus::ACTUAL, {4,4,4,4});
    auto doc2 = "собака злая"s;
    server.AddDocument(2, doc2, DocumentStatus::BANNED, {4,4,4,4});
    auto doc3 = "попугай крикливый нашёлся"s;
    server.AddDocument(1, doc3, DocumentStatus::REMOVED, {5,3,4,1});

    auto query = "кошка злая потерялась"s;
    auto result = server.FindTopDocuments(query);
    // Проверяет счёт релевантности
    ASSERT(abs( result[0].relevance - 0.3662040) < 1e-5);
    // Проверяет счёт рейтинга
    ASSERT_EQUAL(result[0].rating, 4);
    // Проверяет на работу фильтра
    ASSERT(result.size() == 1 && result[0].id == 3);
    }

    {
        SearchServer server(""s);
        auto doc1 = "кошка игривая пропала"s;
        server.AddDocument(1, doc1, DocumentStatus::BANNED, {1,2,3,4});
        try {
            auto doc2 = "кошка злая потерялась"s;
            server.AddDocument(1, doc2, DocumentStatus::BANNED, {4,4,4,4});
            ASSERT(false);
        }  catch (...) {
            auto result = server.FindTopDocuments("кошка злая потерялась"s, DocumentStatus::BANNED);
            // Проверяет на изменение данных, при добавлении документов с одним и тем же индексом
            ASSERT(result.size() == 1 &&
                   result[0].rating == 2);
        }
    }

    {
    SearchServer server(""s);
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(4, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(5, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(6, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(7, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});

    auto result = server.FindTopDocuments("пропала кошка"s);
    // Проверяет на огранчение по количеству в топе
    ASSERT_EQUAL(result.size() , 5);
    }

    {
        SearchServer server("на c"s);
        server.AddDocument(1, "пропался кот"s, DocumentStatus::ACTUAL, {5,3,4,1});
        server.AddDocument(2, "пропала собака"s, DocumentStatus::ACTUAL, {5,3,4,1});
        server.AddDocument(3, "потерялась лиса"s, DocumentStatus::ACTUAL, {5,3,4,1});

        try {
            auto result = server.FindTopDocuments("кот - - - - "s);
            ASSERT(false);
        }  catch (...) {
            // проверка на пустой -
            ASSERT(true);
        }
    }

    {
    SearchServer server("и на c"s);
    auto doc1 = "и кот попался "s;
    server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc2 = "и собака пропала"s;
    server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc3 = "лиса потерялась"s;
    server.AddDocument(3, doc3, DocumentStatus::ACTUAL, {5,3,4,1});

    auto result= server.FindTopDocuments("и лиса"s);
    // Поиск со стоп-словом должен
    ASSERT_EQUAL(result.size() , 1);
    }
}

void TestSetStopWords(){
    {
    SearchServer server("по и или на с до за не"s);

    auto doc1 =  "машина с беспилотным управлением и печкой"s;
    server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    auto doc2 = "автомобиль пропал"s;
    server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {4, 4, 4, 4});


    auto result = server.FindTopDocuments("машина"s);
    ASSERT( abs(result[0].relevance - 0.17328679) < 1e-6);
    }
}

void TestMatchDocument(){
    {
    SearchServer server("и в на");

    auto doc1 = "машина беспилотная"s;
    server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    auto doc2 = "автомобиль пропал"s;
    server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    auto doc3 = "и машина пропала белая"s;
    server.AddDocument(3, doc3, DocumentStatus::ACTUAL, {4, 4, 4, 4});

    auto result1 = server.MatchDocument("машина -беспилотная"s, 1);
    ASSERT_EQUAL(get<0>(result1).size(), 0);

    auto result2 = server.MatchDocument("машина -беспилотная"s, 2);
    ASSERT_EQUAL(get<0>(result2).size(), 0);

    auto result3 = server.MatchDocument("машина -беспилотная пропала"s, 3);
    ASSERT_EQUAL(get<0>(result3).size() , 2);

    try {
        auto result4 = server.MatchDocument("машина -беспилотная пропала"s, 4);
        ASSERT(false);
    }  catch (...) {
        // Выход за пределы id
        ASSERT(true);
    }

    auto result5 = server.MatchDocument("и машина -беспилотная пропала"s, 3);
    ASSERT_EQUAL(get<0>(result5).size() , 2);

    auto result6 = server.MatchDocument("и"s, 3);
    ASSERT_EQUAL(get<0>(result6).size(), 0);
    }

}

void TestSortRelevance(){
    {
        SearchServer server(""s);
        auto doc1 = "робот утилизатор"s;
        server.AddDocument(1,doc1, DocumentStatus::ACTUAL, {5,5,5,5,5});
        auto doc2 = "беспилотный самолёт"s;
        server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {4,4,4,4});
        auto doc3 = "новая посудомойка"s;
        server.AddDocument(3, doc3, DocumentStatus::ACTUAL, {3,3,3});
        auto doc4 = "робот пылесос"s;
        server.AddDocument(4, doc4, DocumentStatus::ACTUAL, {2,2});


        auto result = server.FindTopDocuments("робот утилизатор беспилотный"s);
        ASSERT( result.size() == 3 &&
                result[0].id == 1 &&
                result[1].id == 2 &&
                result[2].id == 4);

        auto doc5 = "робот робот"s;
        server.AddDocument(5, doc5, DocumentStatus::ACTUAL, {1});

        auto result1 = server.FindTopDocuments("робот утилизатор беспилотный"s);
        ASSERT( result1.size() == 4 &&
                result1[0].id == 1 &&
                result1[1].id == 2 &&
                result1[2].id == 5 &&
                result1[3].id == 4);

        auto result2 = server.FindTopDocuments("робот утилизатор беспилотный"s, [](int , DocumentStatus , int rating){
            return rating > 3;
        });
        ASSERT_EQUAL(result2.size() , 2);
    }
}

void TestSplitIntoWords(){

    assert((SplitIntoWords(""s) == vector<string_view>{}));
    assert((SplitIntoWords("     "s) == vector<string_view>{}));
    assert((SplitIntoWords("aaaaaaa"s) == vector{"aaaaaaa"sv}));
    assert((SplitIntoWords("a"s) == vector{"a"sv}));
    assert((SplitIntoWords("a b c"s) == vector{"a"sv, "b"sv, "c"sv}));
    assert((SplitIntoWords("a    bbb   cc"s) == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWords("  a    bbb   cc"s) == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWords("a    bbb   cc   "s) == vector{"a"sv, "bbb"sv, "cc"sv}));
    assert((SplitIntoWords("  a    bbb   cc   "s) == vector{"a"sv, "bbb"sv, "cc"sv}));

    auto doc0 = ""s;
    auto vec1 = SplitIntoWords(doc0);
    ASSERT_EQUAL(vec1.size() , 0);
    auto doc1 = "раз два три"s;
    auto vec2 = SplitIntoWords(doc1);
    ASSERT(vec2.size() == 3 &&
           (vec2[0] == "раз"s));
    auto doc2 = "     раз      два      три     "s;
    auto vec3 = SplitIntoWords(doc2);
    ASSERT(vec3.size() == 3 &&
           vec3[0] == "раз"s);
}

void TestRemoveDocuments(){
    {
    SearchServer server(""s);
    auto doc1 = "робот утилизатор"s;
    server.AddDocument(1, doc1, DocumentStatus::ACTUAL, {5,5,5,5,5});
    auto doc2 = "беспилотный самолёт"s;
    server.AddDocument(2, doc2, DocumentStatus::ACTUAL, {4,4,4,4});
    auto doc3 = "новая посудомойка"s;
    server.AddDocument(3, doc3, DocumentStatus::ACTUAL, {3,3,3});
    auto doc4 = "робот пылесос"s;
    server.AddDocument(4, doc4, DocumentStatus::ACTUAL, {2,2});
    server.RemoveDocument(2);
    ASSERT_EQUAL(server.GetDocumentCount(), 3);
    }
}



void RunConcurrentUpdates(ConcurrentMap<int, int>& cm, size_t thread_count, int key_count) {
    auto kernel = [&cm, key_count](int seed) {
        std::vector<int> updates(key_count);
        iota(begin(updates), end(updates), -key_count / 2);
        shuffle(begin(updates), end(updates), std::mt19937(seed));

        for (int i = 0; i < 2; ++i) {
            for (auto key : updates) {
                ++cm[key].ref_to_value;
            }
        }
    };

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < thread_count; ++i) {
        futures.push_back(std::async(kernel, i));
    }
}

void TestConcurrentUpdate() {
    constexpr size_t THREAD_COUNT = 3;
    constexpr size_t KEY_COUNT = 50000;

    ConcurrentMap<int, int> cm(THREAD_COUNT);
    RunConcurrentUpdates(cm, THREAD_COUNT, KEY_COUNT);

    const auto result = cm.BuildOrdinaryMap();
    ASSERT_EQUAL(result.size(), KEY_COUNT);
    for (auto& [k, v] : result) {
        AssertEqual(v, 6, "Key = " + std::to_string(k));
    }
}

void TestReadAndWrite() {
    ConcurrentMap<size_t, std::string> cm(5);

    auto updater = [&cm] {
        for (size_t i = 0; i < 50000; ++i) {
            cm[i].ref_to_value.push_back('a');
        }
    };
    auto reader = [&cm] {
        std::vector<std::string> result(50000);
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = cm[i].ref_to_value;
        }
        return result;
    };

    auto u1 = std::async(updater);
    auto r1 = std::async(reader);
    auto u2 = std::async(updater);
    auto r2 = std::async(reader);

    u1.get();
    u2.get();

    for (auto f : {&r1, &r2}) {
        auto result = f->get();
        ASSERT(all_of(result.begin(), result.end(), [](const std::string& s) {
            return s.empty() || s == "a" || s == "aa";
        }));
    }
}

void TestSpeedup() {
    {
        ConcurrentMap<int, int> single_lock(1);

        LOG_DURATION("Single lock");
        RunConcurrentUpdates(single_lock, 4, 50000);
    }
    {
        ConcurrentMap<int, int> many_locks(100);

        LOG_DURATION("100 locks");
        RunConcurrentUpdates(many_locks, 4, 50000);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestRunner tr;
    RUN_TEST(tr, TestConcurrentUpdate);
    RUN_TEST(tr, TestReadAndWrite);
    RUN_TEST(tr, TestSpeedup);
    RUN_TEST(tr, TestSplitIntoWords);
    RUN_TEST(tr, TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(tr, TestAddDocument);
    RUN_TEST(tr, TestSetStopWords);
    RUN_TEST(tr, TestMatchDocument);
    RUN_TEST(tr, TestSortRelevance);
    RUN_TEST(tr, TestRemoveDocuments);
    //RUN_TEST(TestGetDocumentId);
}

// --------- Окончание модульных тестов поисковой системы -----------
