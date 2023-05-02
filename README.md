# SearchServer
Программа предоставляет возможность поиска по ключевым словам и ранжирование результатов по статистической мере TF-IDF (TF — term frequency, IDF — inverse document frequency). Поддерживает функциональность стоп-слов. Создание и обработка очереди запросов. Возможность работы в многопоточном режиме.

## Работа с классом поискового сервера:

Создание экземпляра класса SearchServer.В конструктор передаётся строка с стоп-словами, разделенными пробелами. Вместо строки можно передавать произвольный контейнер (с последовательным доступом к элементам с возможностью использования в for-range цикле)

### Обзор методов:
  1. Метод **AddDocument** добавляет документы для поиска. В метод передаётся id документа, статус, рейтинг, и сам документ в формате строки.
  ```c++
  // создаём список стоп слов
  vector<string> stop_words{"и"s, "но"s, "или"s};
  // создаём экземпляр поискового сервера со списком стоп слов
  SearchServer server(stop_words);
  // добавляем документы на сервер
  server.AddDocument(0, "белый кот и пушистый хвост"sv);
  server.AddDocument(1, "черный пёс но желтый хвост"sv);
  server.AddDocument(3, "черный жираф или белый дракон"sv);
  ```
  2. Метод **GetDocumentCount** возвращает количество документов на сервере.
  ``` c++
  cout << "Кол-во документов : "sv << server.GetDocumentCount() << endl;
  ```
  3. Метод **FindTopDocuments** возвращает вектор документов, согласно соответствию переданным ключевым словам. Результаты отсортированы по статистической мере TF-IDF. Возможна дополнительная фильтрация документов по id, статусу и рейтингу. Метод реализован как в однопоточной так и в многпоточной версии.
  ``` c++
  auto result = server.FindTopDocuments("черный дракон"sv);
  cout << "Документы с ключевыми словами \"черный дракон\" : "sv << endl;
  for (auto &doc : result) {
      cout << doc << endl;
  }
  ```
  4. Метод **MatchDocument** производит матчинг запроса и документа по id.
  ``` c++
  for (int document_id : search_server) {
      const auto [words, status] = search_server.MatchDocument(query, document_id);
      PrintMatchDocumentResult(document_id, words, status);
  }
  ```
  5. Метод **RemoveDocument** производит удаление документа по id.
  ``` c++
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
  ```
  6. Метод **GetWordFrequencies** возвращает все слова и их частоту встречаемости в документе по его id.
  ``` c++
  const map<string_view, double>& freqs = search_server.GetWordFrequencies(document_id);
  ```

### Обзор классов:
1. Класс **RequestQueue** реализует хранение истории запросов к поисковому серверу. При этом общее кол-во хранимых запросов не превышает заданного значения. При добавлении новых запросов - они замещают самые старые запросы в очереди.
``` c++
SearchServer search_server("and in at"s);
RequestQueue request_queue(search_server);

search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

// 1439 запросов с нулевым результатом
for (int i = 0; i < 1439; ++i) {
    request_queue.AddFindRequest("empty request"s);
}
// все еще 1439 запросов с нулевым результатом
request_queue.AddFindRequest("curly dog"s);
// новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
request_queue.AddFindRequest("big collar"s);
// первый запрос удален, 1437 запросов с нулевым результатом
request_queue.AddFindRequest("sparrow"s);
cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
```

2. Класс **Paginator** обеспечивает выдачу документов постранично.
``` c++
vector<string> stop_words{"и"s, "но"s, "или"s};
SearchServer server(stop_words);
server.AddDocument(0, "первый документ"sv);
server.AddDocument(1, "второй документ"sv);
server.AddDocument(2, "третий документ"sv);
server.AddDocument(3, "четвертый документ"sv);
server.AddDocument(4, "пятый документ"sv);

const auto search_results = server.FindTopDocuments("документ");

// разбиваем результат поиска на страницы
size_t page_size = 2;
const auto pages = Paginate(search_results, page_size);
for (auto page : pages) {
    cout << page << endl;
    cout << "Page break"s << endl;
}
```
### Обзор функций:
Функции **ProcessQueries** и **ProcessQueriesJoined** обеспечивают параллельное исполнение нескольких запросов к поисковой системе.
```c++
SearchServer search_server("and with"s);

int id = 0;
for (const string& text : {
        "funny pet and nasty rat"s,
        "funny pet with curly hair"s,
        "funny pet and not very nasty rat"s,
        "pet with rat and rat and rat"s,
        "nasty rat with curly hair"s,}) {
            search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
        }

const vector<string> queries = {
    "nasty rat -not"s,
    "not very funny nasty pet"s,
    "curly hair"s
};

id = 0;
for (const auto& documents : ProcessQueries(search_server, queries)) {
    cout << documents.size() << " documents for query ["s << queries[static_cast<size_t>(id++)] << "]"s << endl;
}

cout << endl << endl;
for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
    cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
}
```
## Сборка с помощью CMake
> 1. Клонируйте репозиторий.
> 2.  Создайте папку build для сборки.
> 3. Откройте консоль в папке build и введите в консоли : `cmake ..`.
> 4. Введите команду : `make` или `make -j<количество ядер вашего процессора>`, чтобы сборка происходила быстрее.  
> 5. После сборки в папке build появится исполняемый файл с именем `SearchServer` (Пока `SearchServer` -- только тестовая версия).

## Системные требования
Компилятор С++ с поддержкой стандарта C++17 или новее.
Для сборки многопоточных версий методов необходим Intel TBB.
CMake версии 3.20 и выше.

## TODO:
Написать Cmake файл для сборки проекта. Сделать GUI с использованием QT для работы с поисковым сервером.
Переименовать проект, перенести его в CLion
