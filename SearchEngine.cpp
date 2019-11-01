#include "BinaryMmap.cpp"
#include "ExpToken.cpp"
#include <set>
#include <iostream>

using namespace std;
using DocIdSet = set<unsigned short>;

class SearchEngine {

private:
    DocIdSet *totalDocIdSet;

    BinaryMmap reverseIndex;
    BinaryMmap coordBlocks;

    queue<exp_token> search_rq_tokens;
    exp_token curr_tok {START};
    DocIdSet curr_term_set;

public:

    SearchEngine() :
            reverseIndex(BinaryMmap("reverseIndex.bin", 0)),
            coordBlocks(BinaryMmap("coordBlocks.bin", 0)) {

        static DocIdSet totalDocIdSet;
        if (totalDocIdSet.size() == 0) {
            DocIdSet::iterator iter = totalDocIdSet.end();
            for (int docId = 0; docId <= 40926; docId++)
                totalDocIdSet.insert(iter, docId);
        }

        this->totalDocIdSet = &totalDocIdSet;
    }

    ~SearchEngine() {
        coordBlocks.terminate();
        reverseIndex.terminate();
    }

    struct index_termin {
        string value;
        unsigned short docCount;
        size_t offset;
    };

    DocIdSet search(string rq) {
        search_rq_tokens = analizator::analize_request(rq);
        getToken();
        return expr();
    }

private:

    DocIdSet expr() {
        DocIdSet left = term();

        DocIdSet result;
        while (1) {
            if (curr_tok.type == OR) {
                getToken();
                DocIdSet right = term();
                set_union(left.begin(), left.end(), right.begin(), right.end(), inserter(result, result.begin()));
                left = result;
                result.clear();
            } else {
                return left;
            }
        }
    }

    DocIdSet term() {
        DocIdSet left = prim();
        DocIdSet right;
        DocIdSet result;
        while (1) {
            switch (curr_tok.type) {
                case AND:
                    getToken();
                    right = prim();
                    set_intersection(left.begin(), left.end(), right.begin(), right.end(), inserter(result, result.begin()));
                    left = result;
                    result.clear();
                    break;
                case AND_NOT:
                    getToken();
                    right = prim();
                    set_difference(left.begin(), left.end(), right.begin(), right.end(), inserter(result, result.begin()));
                    left = result;
                    result.clear();
                    break;
                default:
                    return left;
            }
        }
    }

    DocIdSet prim() {
        DocIdSet result;
        DocIdSet e;
        switch (curr_tok.type) {
            case TERM:
                getToken();
                return curr_term_set;
            case NOT:
                getToken();
                e = expr();
                set_difference((*totalDocIdSet).begin(), (*totalDocIdSet).end(), e.begin(), e.end(), inserter(result, result.begin()));
                return result;
            case LP:
                getToken();
                e = expr();
                if (curr_tok.type != RP) {
                    cout << "Error happened, expresion is missing ')'";
                    return result;
                }
                getToken();
                return e;
            default:
                return result;
        }
    }

    exp_token getToken() {
        curr_tok = search_rq_tokens.front();
        if (curr_tok.type == TERM)
            curr_term_set = find(curr_tok.data);
        search_rq_tokens.pop();
        return curr_tok;
    }

    DocIdSet find(string search_term) {
        if (search_term.size() == 0)
            return DocIdSet();

        reverseIndex.updateCurrentPosition();

        DocIdSet result;
        index_termin cur_term;
        size_t offset = 0;

        while (cur_term.value != search_term) {
            int len = reverseIndex.readInt(1, offset);
            if (!len)
                return result;
            cur_term.value = reverseIndex.readStr(len);
            cur_term.docCount = reverseIndex.readInt(2);
            offset = reverseIndex.currentPosition() + 4;
            cur_term.offset = reverseIndex.readInt(4, offset);
            offset = reverseIndex.currentPosition();
            if (cur_term.value > search_term) {
                offset += 4;
            } else if (cur_term.value < search_term) {
                offset = reverseIndex.readInt(4);
                if (!offset)
                    return result;
            }
        }

        coordBlocks.updateCurrentPosition(cur_term.offset);
        auto it = result.end();
        for (int i = 0; i < cur_term.docCount; ++i)
            result.insert(it, coordBlocks.readInt(2));


        return result;
    }

};