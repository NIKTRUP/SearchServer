#ifndef TEST_EXAMPLE_FUNCTIONS_H
#define TEST_EXAMPLE_FUNCTIONS_H
#include "search_server.h"
//#include "write_output_functions.h"
#include "remove_duplicates.h"
#include <cassert>


template <typename Function>
void RunTestImpl(Function function,const std::string& func) {
    function();
    std::cerr << func << " OK" << std::endl;
}

#define RUN_TEST(func)  RunTestImpl( (func) , #func )

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u,
                     const std::string& t_str, const std::string& u_str,
                     const std::string& file, const std::string& func,
                     unsigned line, const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << file << "(" << line << "): " << func << ": ";
        std::cerr << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: ";
        std::cerr << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cerr << " Hint: " << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str,
                const std::string& file, const std::string& func,
                unsigned line, const std::string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

std::vector<int> TakeEvens(const std::vector<int>& numbers);

std::map<std::string, int> TakeAdults(const std::map<std::string, int>& people);

bool IsPrime(int n);

std::set<int> TakePrimes(const std::set<int>& numbers);


// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent();

void TestAddDocument();

void TestSetStopWords();

void TestMatchDocument();

void TestSortRelevance();

void TestSplitIntoWords();

//void TestGetDocumentId();

void TestRemoveDocuments();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();
// --------- Окончание модульных тестов поисковой системы -----------


#endif // TEST_EXAMPLE_FUNCTIONS_H
