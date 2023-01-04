#ifndef TEST_EXAMPLE_FUNCTIONS_H
#define TEST_EXAMPLE_FUNCTIONS_H
#include "search_server.h"
#include "log_duration.h"
#include "test_framework.h"
#include "concurrent_map.h"
#include <cassert>

using namespace std;


void PrintDocument(const Document& document);
void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status);
void AddDocument(SearchServer& search_server, int document_id, std::string_view document, DocumentStatus status,
                 const std::vector<int>& ratings);
void FindTopDocuments(const SearchServer& search_server, std::string_view raw_query);
void MatchDocuments(const SearchServer& search_server, std::string_view query);
void RemoveDuplicates(SearchServer& search_server);


// -------- Начало модульных тестов поисковой системы ----------

void TestExcludeStopWordsFromAddedDocumentContent();

void TestAddDocument();

void TestSetStopWords();

void TestMatchDocument();

void TestSortRelevance();

void TestSplitIntoWords();

void TestRemoveDocuments();

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();

// --------- Окончание модульных тестов поисковой системы -----------


#endif // TEST_EXAMPLE_FUNCTIONS_H
