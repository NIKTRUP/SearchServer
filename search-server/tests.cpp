#include "tests.h"

using namespace std::chrono_literals;
using namespace std;

void AssertImpl(bool value, const std::string& expr_str,
                const std::string& file, const std::string& func,
                unsigned line, const std::string& hint) {
    if (!value) {
        std::cerr << file << "("s << line << "): "s << func << ": "s;
        std::cerr << "Assert("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

std::vector<int> TakeEvens(const std::vector<int>& numbers) {
    std::vector<int> evens;
    for (int x : numbers) {
        if (x % 2 == 0) {
            evens.push_back(x);
        }
    }
    return evens;
}

std::map<std::string, int> TakeAdults(const std::map<std::string, int>& people) {
    std::map<std::string, int> adults;
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

std::set<int> TakePrimes(const std::set<int>& numbers) {
    std::set<int> primes;
    for (int number : numbers) {
        if (IsPrime(number)) {
            primes.insert(number);
        }
    }
    return primes;
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
        auto doc = "пропала кошка"s;
        server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
        doc = "а"s;
        server.AddDocument(2, doc, DocumentStatus::BANNED, {5,3,4,1});
        doc = "б"s;
        server.AddDocument(3, doc, DocumentStatus::IRRELEVANT, {5,3,4,1});
        doc = "с"s;
        server.AddDocument(4, doc, DocumentStatus::REMOVED, {5,3,4,1});
        auto result = server.FindTopDocuments("пропала кошка"s);
        ASSERT_EQUAL_HINT(result.size() , 1,
                          "Проверяет на добавление документов с разными статусами"s);
    }

    {
    SearchServer server(""s);
    auto doc = "пропала кошка"s;
    server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
    auto doc2 = "пропала кошка"s;
    auto result = server.FindTopDocuments(doc2);
    ASSERT_HINT(result[0].relevance == 0 &&
           result[0].id == 1 &&
           result[0].rating == 13/4 ,
           "Проверяет на добавление одного документа и вычисление его рейтинга"s);
    }

    {
    SearchServer server(""s);
    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(0, doc, DocumentStatus::ACTUAL, {5,3,4,1});
    }  catch (...) {
        ASSERT_HINT(false,
               "Проверяет на добавление id == 0"s);
    }

    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(-1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        ASSERT_HINT(true,
               "Проверяет на добавление отрицательного id"s);
    }

    try {
        auto doc = "пропала кошка"s;
        server.AddDocument(-2, doc , DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        ASSERT_HINT(true,
               "Проверяет на добавление отрицательного id"s);
        }
    }

    {
    SearchServer server(""s);
    auto doc = "пропала кошка"s;
    server.AddDocument(0, doc, DocumentStatus::ACTUAL, {5,3,4,1});
    doc = "пропала собака"s;
    server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});

    try {
        auto doc ="пропала ящерица"s;
        server.AddDocument(0, doc, DocumentStatus::ACTUAL, {5,3,4,1});
        doc =  "пропал попугай"s;
        server.AddDocument(1, doc, DocumentStatus::ACTUAL, {5,3,4,1});
        ASSERT(false);
    }  catch (...) {
        auto doc2 =  "пропала кошка"s;
        auto result = server.FindTopDocuments(doc2);
        ASSERT_EQUAL_HINT(result.size(), 2,
               "Проверяет на добавление документа с существующим id"s);
        }
    }



    {
        SearchServer server(""s);
        try {
            server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2});
            ASSERT(false);
        }  catch (...) {
            ASSERT_HINT(true, "Проверяем на добавление документа со спецсимволами"s);
        }

    }

    {
    SearchServer server(""s);
    (void) server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});


    auto result = server.FindTopDocuments("кошка пропала"s);
    ASSERT_HINT(result.size() == 1 &&
           result[0].relevance == 0 &&
           result[0].id == 1 &&
           result[0].rating == 13/4 ,
            "Проверяет, что результат не зависит от перемены мест слов"s);
    }

    {
    SearchServer server(""s);
    server.AddDocument(1, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "пропала собака"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "потерялась лисица"s, DocumentStatus::ACTUAL, {5,3,4,1});

    auto result = server.FindTopDocuments("лисица -пропала"s);
    ASSERT_EQUAL_HINT(result.size() , 1,
                      "Проверяет на исключение документа из списка выдачи"s);
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
        ASSERT_HINT(true, "Проверяет на обработку --"s);
    }


    }

    {
    SearchServer server(""s);
    server.AddDocument(3, "кошка игривая пропала"s, DocumentStatus::ACTUAL, {4,4,4,4});
    server.AddDocument(2, "собака злая"s, DocumentStatus::BANNED, {4,4,4,4});
    server.AddDocument(1, "попугай крикливый нашёлся"s, DocumentStatus::REMOVED, {5,3,4,1});

    auto result = server.FindTopDocuments("кошка злая потерялась"s);
    ASSERT_HINT(abs( result[0].relevance - 0.3662040) < 1e-5, "Проверяет счёт релевантности");
    ASSERT_EQUAL_HINT(result[0].rating, 4, "Проверяет счёт рейтинга");
    ASSERT_HINT(result.size() == 1 && result[0].id == 3, "Проверяет на работу фильтра"s);
    }

    {
        SearchServer server(""s);
        server.AddDocument(1, "кошка игривая пропала"s, DocumentStatus::BANNED, {1,2,3,4});
        try {
            server.AddDocument(1, "кошка злая потерялась"s, DocumentStatus::BANNED, {4,4,4,4});
            ASSERT(false);
        }  catch (...) {
            auto result = server.FindTopDocuments("кошка злая потерялась"s, DocumentStatus::BANNED);
            ASSERT_HINT(result.size() == 1 &&
                   result[0].rating == 2,
                   "Проверяет на изменение данных, при добавлении документов с одним и тем же индексом"s );
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
    ASSERT_EQUAL_HINT(result.size() , 5,
                      "Проверяет на огранчение по количеству в топе"s);
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
            ASSERT_HINT(true, "проверка на пустой -"s);
        }
    }

    {
    SearchServer server("и на c"s);
    server.AddDocument(1, "и кот попался "s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(2, "и собака пропала"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(3, "лиса потерялась"s, DocumentStatus::ACTUAL, {5,3,4,1});

    auto result= server.FindTopDocuments("и лиса"s);
    ASSERT_EQUAL_HINT(result.size() , 1,
                      "Поиск со стоп-словом должен"s);
    }
}

void TestSetStopWords(){
    {
    SearchServer server("по и или на с до за не"s);

    server.AddDocument(1, "машина с беспилотным управлением и печкой"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(2, "автомобиль пропал"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});


    auto result = server.FindTopDocuments("машина"s);
    ASSERT( abs(result[0].relevance - 0.17328679) < 1e-6);
    }
}

void TestMatchDocument(){
    {
    SearchServer server("и в на");

    server.AddDocument(1, "машина беспилотная"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(2, "автомобиль пропал"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});
    server.AddDocument(3, "и машина пропала белая"s, DocumentStatus::ACTUAL, {4, 4, 4, 4});

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
        ASSERT_HINT(true, "Выход за пределы id"s);
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
        server.AddDocument(1, "робот утилизатор"s, DocumentStatus::ACTUAL, {5,5,5,5,5});
        server.AddDocument(2, "беспилотный самолёт"s, DocumentStatus::ACTUAL, {4,4,4,4});
        server.AddDocument(3, "новая посудомойка"s, DocumentStatus::ACTUAL, {3,3,3});
        server.AddDocument(4, "робот пылесос"s, DocumentStatus::ACTUAL, {2,2});


        auto result = server.FindTopDocuments("робот утилизатор беспилотный"s);
        ASSERT( result.size() == 3 &&
                result[0].id == 1 &&
                result[1].id == 2 &&
                result[2].id == 4);

        server.AddDocument(5, "робот робот"s, DocumentStatus::ACTUAL, {1});

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
    auto vec1 = SplitIntoWords(""s);
    ASSERT_EQUAL(vec1.size() , 0);
    auto vec2 = SplitIntoWords("раз два три"s);
    ASSERT(vec2.size() == 3 &&
           vec2[0] == "раз"s);
    auto vec3 = SplitIntoWords("     раз      два      три     "s);
    ASSERT(vec3.size() == 3 &&
           vec3[0] == "раз"s);
}

void TestGetDocumentId(){
    SearchServer server(""s);
    server.AddDocument(0, "пропала кошка"s, DocumentStatus::ACTUAL, {5,3,4,1});
    server.AddDocument(1, "а"s, DocumentStatus::BANNED, {5,3,4,1});
    server.AddDocument(2, "б"s, DocumentStatus::IRRELEVANT, {5,3,4,1});
    server.AddDocument(3, "с"s, DocumentStatus::REMOVED, {5,3,4,1});
    try {
        server.GetDocumentId(-1);
        ASSERT(false);
    }  catch (...) {
        ASSERT(true);
    }

    try {
        server.GetDocumentId(5);
        ASSERT(false);
    }  catch (...) {
        ASSERT(true);
    }
    ASSERT_EQUAL(server.GetDocumentId(2), 2);
    ASSERT_EQUAL(server.GetDocumentId(3), 3);
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestSetStopWords);
    RUN_TEST(TestMatchDocument);
    RUN_TEST(TestSortRelevance);
    RUN_TEST(TestSplitIntoWords);
    RUN_TEST(TestGetDocumentId);
}

// --------- Окончание модульных тестов поисковой системы -----------
