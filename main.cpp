#include "SearchEngine.cpp"

using namespace std;
int main() {

    SearchEngine searchEngine;
    string search_rq;

    cout << "Enter search request" << endl;
    getline(cin, search_rq);

    DocIdSet result = searchEngine.search(search_rq);
    for (auto docId : result)
        cout << docId << "; ";
    cout << endl;

    return 0;
}