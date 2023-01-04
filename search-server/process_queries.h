#ifndef PROCESS_QUERIES_H
#define PROCESS_QUERIES_H

#include "document.h"
#include "search_server.h"
#include <string>
#include <vector>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries);

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries);

#endif // PROCESS_QUERIES_H
