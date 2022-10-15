#define DEBUG
#include "paginator.h"
#include "search_server.h"
#include "request_queue.h"
#include "read_input_functions.h"
#include "tests.h"

using namespace std::literals::string_literals;

int main() {

#ifdef DEBUG
    TestSearchServer();
    std::cerr << "Search server testing finished"s << std::endl;
#endif

    auto sbc = "and in at"s;
    SearchServer search_server(sbc);
    RequestQueue request_queue(search_server);
     sbc = "curly cat curly tail"s;
    auto abc1 = {7, 2, 7};
    search_server.AddDocument(1, sbc, DocumentStatus::ACTUAL, abc1);
    sbc = "curly dog and fancy collar"s;
    std::vector<int> abc2 = {1, 2, 3};
    search_server.AddDocument(2, sbc, DocumentStatus::ACTUAL,abc2 );
    sbc = "big cat fancy collar "s;
    std::vector<int> abc3 = {1, 2, 8};
    search_server.AddDocument(3, sbc, DocumentStatus::ACTUAL, abc3);
     sbc = "big dog sparrow Eugene"s;
    std::vector<int> abc4 = {1, 3, 2};
    search_server.AddDocument(4, sbc, DocumentStatus::ACTUAL, abc4);
    sbc = "big cat fancy collar "s;
    std::vector<int> abc5 = {1, 1, 1};
    search_server.AddDocument(5, sbc, DocumentStatus::ACTUAL, abc5);
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    std::string doc2 = "curly dog"s;
    request_queue.AddFindRequest(doc2);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    doc2 = "big collar"s;
    request_queue.AddFindRequest(doc2);
    // первый запрос удален, 1437 запросов с нулевым результатом
    doc2 = "sparrow"s;
    request_queue.AddFindRequest(doc2);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    return 0;

}
