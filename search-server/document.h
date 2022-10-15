#ifndef DOCUMENT_H
#define DOCUMENT_H
#include <iostream>


struct Document {

    Document();
    Document(int id_, double relevance_, int rating_);

    int id;
    double relevance;
    int rating;
};

std::ostream& operator<<(std::ostream& out, const Document& document);

void PrintDocument(const Document& document);

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

#endif // DOCUMENT_H
