#include <regex>
#include <queue>
#include <map>
#include <regex>

using namespace std;

enum token_value {
    OR, AND, AND_NOT, NOT = '!', LP = '(', RP = ')', TERM, START, END
};

struct exp_token {
    token_value type;
    string data;
};

namespace analizator {
    string &lowerCaseStr(string &str);

    queue<exp_token> analize_request(string search_rq) {
        queue<exp_token> search_rq_tokens;

        regex tok_rx(
                "(?:(?:&{2})| |(?:\\|{2})|!|\\(|\\)|(?:\\w|\\xD0[\\x80-\\xBF]|\\xD1[\\x80-\\x9F]|\\xCC[\\x80-\\xBB])+)");
        regex grave_rx("\\xCC[\\x80-\\xBB]");


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
                t = regex_replace(t, grave_rx, "");
                lowerCaseStr(t);
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

    string &lowerCaseStr(string &str) {
        unsigned char *main = (unsigned char *)&(str[0]);
        unsigned char *head = 0;

        while (*main) {
            if ((*main >= 0x41) && (*main <= 0x5a)) { // US ASCII
                (*main) += 0x20;
            } else if (*main > 0xc0) {
                head = main;
                main++;
                switch (*head) {
                    case 0xd0: // Cyrillic
                        if ((*main >= 0x80) && (*main <= 0x8f)) {
                            *head = 0xd1;
                            (*main) += 0x10;
                        } else if ((*main >= 0x90) && (*main <= 0x9f))
                            (*main) += 0x20; // US ASCII shift
                        else if ((*main >= 0xa0) && (*main <= 0xaf)) {
                            *head = 0xd1;
                            (*main) -= 0x20;
                        }
                        break;
                    case 0xd1: // Cyrillic supplement
                        if ((*main >= 0xa0) && (*main <= 0xbf) && (!(*main % 2))) // Even
                            (*main)++;
                        break;
                }
                head = 0;
            }
            main++;
        }
        return str;
    }
}