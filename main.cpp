#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <map>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
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
        }
        else {
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
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        document_count_++;
        const vector<string> words = SplitIntoWordsNoStop(document);
        double word_term_frequency = 1.0 / words.size();
        for (const string& word : words) {
            documents_[word].insert(document_id);
            words_term_freq[word][document_id] += word_term_frequency;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    map<string, set<int>> documents_;
    set<string> stop_words_;
    map<string, map<int, double>> words_term_freq;
    int document_count_ = 0;

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

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

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word.substr(0, 1) == "-") {
                query_words.minus_words.insert(word.substr(1));
            }
            else {
                query_words.plus_words.insert(word);
            }
        }
        return query_words;
    }

    double Find_IDF(const string& word) const {
        return log(document_count_ * 1.0 / words_term_freq.at(word).size());
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> docunents_rel;//key — document's id, value — relevance
        vector<Document> matched_documents;

        for (const auto& [word, documents_id] : documents_) {
            if (query_words.plus_words.count(word) != 0) {
                double inverse_doc_freq = Find_IDF(word);
                for (const auto& [id, term_freq] : words_term_freq.at(word)) {
                    docunents_rel[id] += term_freq * inverse_doc_freq;
                }
            }
        }

        for (const auto& [word, documents_id] : documents_) {
            if (query_words.minus_words.count(word) != 0) {
                for (const int& id : documents_id) {
                    docunents_rel.erase(id);
                }
            }
        }

        for (const auto& [id, rel] : docunents_rel) {
            matched_documents.push_back({ id, rel });
        }

        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
	const int document_count = ReadLineWithNumber();
    
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}


int main() {
	const SearchServer search_server = CreateSearchServer();

	const string query = ReadLine();
	for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
		cout << "{ document_id = "s << document_id << ", "
			<< "relevance = "s << relevance << " }"s << endl;
	}
}