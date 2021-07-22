#include <queue>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;

void decode_JSON(string, vector<string> &, vector<string> &);

void
add_JSON(string, int, bool, vector<string> &, unordered_map<string, int> &,
         vector<unordered_map<string, vector<int>>> &);

void
get_JSON(string, bool, vector<string> &, unordered_map<string, int> &, vector<unordered_map<string, vector<int>>> &);

void
delete_JSON(string, bool, vector<string> &, unordered_map<string, int> &, vector<unordered_map<string, vector<int>>> &);

vector<int> getIntersection(vector<vector<int> > &);

int main() {
    vector<string> storage;
    string input;
    bool ifList = false;
    unordered_map<string, int> lookup;
    vector<unordered_map<string, vector<int>>> search;
    int entryID = 0;

    while (getline(cin, input)) {
        // check the input type
        ifList = false;
        if (input.find("list") != string::npos)
            ifList = true;
        switch (input[0]) {
            case 'a':
                add_JSON(input.substr(input.find('{')), entryID, ifList, storage, lookup, search);
                entryID++;
                break;
            case 'g':
                get_JSON(input.substr(input.find('{')), ifList, storage, lookup, search);
                break;
            case 'd':
                delete_JSON(input.substr(input.find('{')), ifList, storage, lookup, search);
                break;
            default:
                cerr << "error." << endl;
                exit(1);
        }
    }
    return 0;
}

// use decode_JSON to parse JSON information into keywords of categories(key) and contents(objects)
// (I prefer my implementation to the C++ JSON library)
void decode_JSON(string input, vector<string> &categories, vector<string> &contents) {
    int i = 0;
    string key, value, temp;
    bool ifKey = false;
    size_t end_quote;
    while (i < (int) input.length()) {
        switch (input[i]) {
            case '{':
                ifKey = true;
                break;
            case ',':
                if (value.compare("")) {
                    categories.push_back(key);
                    contents.push_back(value);
                }
                value = "";
                ifKey = true;
                break;
            case '}':
                if (value.compare("")) {
                    categories.push_back(key);
                    contents.push_back(value);
                }
                value = "";
                ifKey = true;
                break;
            case '\"':
                end_quote = input.find('\"', i + 1);
                temp = input.substr(i + 1, end_quote - i - 1);
                if (ifKey)
                    key = temp;
                else
                    value = '\"' + temp + '\"';
                i = (int) end_quote;
                break;
            case ':':
                ifKey = false;
                break;
            case ' ':
                break;
            default:
                if (!ifKey)
                    value.push_back(input[i]);
                break;
        }
        i++;
    }
}

// use add_JSON to add a JSON query into the search hashmap
void add_JSON(string input, int entryID, bool ifList, vector<string> &storage, unordered_map<string, int> &lookup,
              vector<unordered_map<string, vector<int>>> &search) {
    storage.push_back(input);
    if (!ifList) {
        vector<string> categories;
        vector<string> contents;

        decode_JSON(input, categories, contents);

        if (lookup.empty()) {
            search.resize((int) categories.size() + 1);
            for (int j = 0; j != (int) categories.size(); ++j)
                lookup[categories[j]] = j;
        }

        for (int j = 0; j != (int) categories.size(); ++j)
            search[lookup[categories[j]]][contents[j]].push_back(entryID);

    } else {
        int i = (int) (input.find('[')) + 1;
        if (search.empty())
            search.resize(1);
        string value;
        while (input[i] != ']') {
            if (input[i] == ',') {
                search[0][value].push_back(entryID);
                value = "";
            } else
                value.push_back(input[i]);
            i++;
        }
        search[0][value].push_back(entryID);
    }
}

// use get_JSON to get suitable JSON queries
void get_JSON(string input, bool ifList, vector<string> &storage, unordered_map<string, int> &lookup,
              vector<unordered_map<string, vector<int>>> &search) {
    if (!ifList) {
        if (lookup.empty())
            return;

        vector<string> categories;
        vector<string> contents;

        decode_JSON(input, categories, contents);
        vector<vector<int>> partial_results(categories.size());
        for (int j = 0; j != (int) categories.size(); ++j) {
            int lookup_number = lookup[categories[j]];
            if (search[lookup_number].find(contents[j]) != search[lookup_number].end()) {
                for (auto t: search[lookup[categories[j]]][contents[j]])
                    partial_results[j].push_back(t);
            } else
                return;
        }

        if (!partial_results.empty()) {
            vector<int> full_results = getIntersection(partial_results);
            for (auto c: full_results)
                if (storage[c] != "null")
                    cout << storage[c] << "\n";
        } else {
            // the case of get{}
            for (auto c: storage)
                if (c != "null")
                    cout << c << "\n";
        }
    } else {
        int i = (int) (input.find('[')) + 1;
        vector<vector<int>> partial_results;
        string value;
        while (input[i] != ']') {
            if (input[i] == ',') {
                if (search[0].find(value) != search[0].end()) {
                    partial_results.push_back(search[0][value]);
                } else
                    return;
                value = "";
            } else
                value.push_back(input[i]);
            i++;
        }
        if (search[0].find(value) != search[0].end()) {
            partial_results.push_back(search[0][value]);
        } else
            return;

        if (!partial_results.empty()) {
            vector<int> full_results = getIntersection(partial_results);
            for (auto c: full_results)
                if (storage[c] != "null")
                    cout << storage[c] << "\n";
        } else {
            // the case of delete{}
            for (auto c: storage)
                if (c != "null")
                    cout << c << "\n";
        }
    }
}

// use delete_JSON to delete a JSON query from the search hashmap
void delete_JSON(string input, bool ifList, vector<string> &storage, unordered_map<string, int> &lookup,
                 vector<unordered_map<string, vector<int>>> &search) {
    if (!ifList) {
        if (lookup.empty())
            return;

        vector<string> categories;
        vector<string> contents;

        decode_JSON(input, categories, contents);
        vector<vector<int>> partial_results(categories.size());
        for (int j = 0; j != (int) categories.size(); ++j) {
            int lookup_number = lookup[categories[j]];
            if (search[lookup_number].find(contents[j]) != search[lookup_number].end()) {
                for (auto t: search[lookup[categories[j]]][contents[j]])
                    partial_results[j].push_back(t);
            } else
                return;
        }

        if (!partial_results.empty()) {
            vector<int> full_results = getIntersection(partial_results);
            for (auto c: full_results)
                storage[c] = "null";
        } else {
            // the case of delete{}
            for (auto &c: storage)
                c = "null";
        }

    } else {
        int i = (int) (input.find('[')) + 1;
        vector<vector<int>> partial_results;
        string value;
        while (input[i] != ']') {
            if (input[i] == ',') {
                if (search[0].find(value) != search[0].end()) {
                    partial_results.push_back(search[0][value]);
                } else
                    return;
                value = "";
            } else
                value.push_back(input[i]);
            i++;
        }
        if (search[0].find(value) != search[0].end()) {
            partial_results.push_back(search[0][value]);
        } else
            return;

        if (!partial_results.empty()) {
            vector<int> full_results = getIntersection(partial_results);
            for (auto c: full_results)
                storage[c] = "null";
        } else {
            // the case of delete{}
            for (auto c: storage)
                c = "null";
        }
    }
}

// use getIntersection to combine the search results of each item
vector<int> getIntersection(vector<vector<int>> &sets) {
    vector<int> result;
    int smallest = 0;
    int minSize = (int) sets[0].size();

    if (sets.size() == 1) {
        vector<int>::iterator temp;
        temp = unique(sets[0].begin(), sets[0].end());
        sets[0].resize((unsigned) (temp - sets[0].begin()));
        return sets[0];
    }

    for (int i = 1; i != (int) sets.size(); ++i) {
        if (minSize > (int) sets[i].size()) {
            minSize = (int) sets[i].size();
            smallest = i;
        }
    }

    vector<int>::iterator temp;
    temp = unique(sets[smallest].begin(), sets[smallest].end());
    sets[smallest].resize((unsigned) (temp - sets[smallest].begin()));

    vector<int> elements;
    for (auto c: sets[smallest])
        elements.push_back(c);

    for (auto it = elements.begin(); it != elements.end(); ++it) {
        bool ifFound = true;

        for (int j = 0; j != (int) sets.size(); ++j) {
            if (j != smallest) {
                if (!binary_search(sets[j].begin(), sets[j].end(), *it)) {
                    ifFound = false;
                    break;
                }
            }
        }

        if (ifFound)
            result.push_back(*it);
    }
    return result;
}