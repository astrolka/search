#include "SearchEngine.cpp"
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
int main() {

    SearchEngine searchEngine;
    string path;

    cout << "Enter path to file with requests..." << endl;
    cin >> path;

    ifstream infile(path);

    string search_rq;
    while (getline(infile, search_rq)) {
        cout << search_rq;
        high_resolution_clock::time_point search_timer = high_resolution_clock::now();
        DocIdSet result = searchEngine.search(search_rq);
        high_resolution_clock::time_point now = high_resolution_clock::now();
        size_t time = duration_cast<microseconds>(now - search_timer).count();
        cout << ": " << time << " microseconds" << endl;
        for (auto docId : result)
            cout << docId << "; ";
        cout << endl;
    }

    return 0;
}