#include <regex>
#include <queue>
#include <map>

using namespace std;

enum token_value {
    OR, AND, AND_NOT, NOT = '!', LP = '(', RP = ')', TERM, START, END
};

struct exp_token {
    token_value type;
    string data;
};

namespace analizator {
    queue<exp_token> analize_request(string search_rq) {
        queue<exp_token> search_rq_tokens;

        regex tok_rx(
                "(?:(?:&{2})| |(?:\\|{2})|!|\\(|\\)|(?:\\w|\\xD0[\\x80-\\xBF]|\\xD1[\\x80-\\x9F]|\\xCC[\\x80-\\xBB])+)");

        static map<string, token_value> op{{"&&", AND},
                                           {" ",  AND},
                                           {"||", OR},
                                           {"!",  NOT},
                                           {"(",  LP},
                                           {")",  RP}};

        for (sregex_iterator it = sregex_iterator(search_rq.begin(), search_rq.end(), tok_rx); it != sregex_iterator(); ++it) {
            string t = (*it).str();
            auto pos = op.find(t);

            if (pos == op.end()) {
                search_rq_tokens.push({TERM, t});
            } else {
                token_value tv = pos->second;

                token_value prev_tv = START;
                if (search_rq_tokens.size())
                    prev_tv = search_rq_tokens.back().type;

                if (tv == AND && (prev_tv == TERM || prev_tv == RP))
                    search_rq_tokens.push({tv});
                else if (tv == OR && prev_tv == AND)
                    search_rq_tokens.back().type = OR;
                else if (tv == NOT && prev_tv == AND)
                    search_rq_tokens.back().type = AND_NOT;
                else if (tv == NOT || tv == LP || tv == RP || tv == OR)
                    search_rq_tokens.push({tv});
            }
        }

        search_rq_tokens.push({END});

        return search_rq_tokens;
    }
}