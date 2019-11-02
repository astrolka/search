#include "SearchEngine.cpp"
#include <fstream>

using namespace std;
int main() {

    SearchEngine searchEngine;
    string path;

    cout << "Enter path to file with requests..." << endl;
    cin >> path;

    ifstream infile(path);

    string search_rq;
    while (getline(infile, search_rq)) {
        cout << search_rq << endl;
        DocIdSet result = searchEngine.search(search_rq);
        for (auto docId : result)
            cout << docId << "; ";
        cout << endl;
    }

    return 0;
}