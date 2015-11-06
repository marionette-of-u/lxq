#include "automaton_lexer.hpp"
#include "common.hpp"

namespace regex_parser{
    struct token{
        char c;
        int type;
    };

    struct node{
        enum op_type_enum{
            op_or,
            op_dot,
            op_concat,
            op_kleene,
            op_kleene_plus,
            op_one_or_zero,
            op_seq,
            op_range,
            op_class,
            op_charactor
        };

        char c;
        op_type_enum op_type;
        std::vector<node*> node_vec;

        std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const{
            std::size_t r;
            switch(op_type){
            case op_charactor:
                {
                    pool.push_back(automaton::node());
                    pool[start].edge.push_back(std::make_pair(c, pool.size() - 1));
                    r = pool.size() - 1;
                }
                break;

            case op_class:
                {
                    pool.push_back(automaton::node());
                    r = pool.size() - 1;
                    for(std::size_t i = 0; i < node_vec.size(); ++i){
                        for(std::size_t j = 0; j < node_vec[i]->node_vec.size(); ++j){
                            if(node_vec[i]->node_vec[j]->op_type == op_charactor){
                                pool[start].edge.push_back(std::make_pair(node_vec[i]->node_vec[j]->c, r));
                            }else if(node_vec[i]->node_vec[j]->op_type == op_range){
                                if(node_vec[i]->node_vec[j]->node_vec[0]->c > node_vec[i]->node_vec[j]->node_vec[1]->c){
                                    std::swap(node_vec[i]->node_vec[j]->node_vec[0]->c, node_vec[i]->node_vec[j]->node_vec[1]->c);
                                }

                                for(char c = node_vec[i]->node_vec[j]->node_vec[0]->c; c <= node_vec[i]->node_vec[j]->node_vec[1]->c; ++c){
                                    if(c == 0){ continue; }
                                    pool[start].edge.push_back({});
                                    pool[start].edge.back().first = c;
                                    pool[start].edge.back().second = r;
                                }
                            }
                        }
                    }
                }
                break;

            case op_one_or_zero:
                {
                    pool.push_back(automaton::node());
                    r = pool.size() - 1;
                    std::size_t n = node_vec[0]->to_NFA(start, pool);
                    pool[n].edge.push_back(std::make_pair('\0', r));
                    pool[start].edge.push_back(std::make_pair('\0', r));
                }
                break;

            case op_kleene_plus:
                {
                    std::size_t q = node_vec[0]->to_NFA(start, pool);
                    r = node_vec[0]->to_NFA(q, pool);
                    pool[q].edge.push_back(std::make_pair('\0', r));
                    pool[r].edge.push_back(std::make_pair('\0', q));
                }
                break;

            case op_kleene:
                {
                    r = node_vec[0]->to_NFA(start, pool);
                    pool[start].edge.push_back(std::make_pair('\0', r));
                    pool[r].edge.push_back(std::make_pair('\0', start));
                }
                break;

            case op_concat:
                {
                    r = node_vec[0]->to_NFA(start, pool);
                    r = node_vec[1]->to_NFA(r, pool);
                }
                break;

            case op_or:
                {
                    std::size_t e1 = node_vec[0]->to_NFA(start, pool);
                    std::size_t e2 = node_vec[1]->to_NFA(start, pool);
                    pool.push_back(automaton::node());
                    r = pool.size() - 1;
                    pool[e1].edge.push_back(std::make_pair('\0', r));
                    pool[e2].edge.push_back(std::make_pair('\0', r));
                }
                break;
            }
            return r;
        }

        ~node(){
            for(auto &i : node_vec){
                delete i;
            }
        }
    };

    struct semantic_action{
        static node *make_identity(node *ptr){
            return ptr;
        }

        static node *make_or(node *p, node *q){
            node *r = new node;
            r->op_type = node::op_or;
            r->node_vec.push_back(p);
            r->node_vec.push_back(q);
            return r;
        }

        static node *make_concat(node *p, node *q){
            node *r = new node;
            r->op_type = node::op_concat;
            r->node_vec.push_back(p);
            r->node_vec.push_back(q);
            return r;
        }

        static node *make_kleene(node *p){
            node *r = new node;
            r->op_type = node::op_kleene;
            r->node_vec.push_back(p);
            return r;
        }

        static node *make_kleene_plus(node *p){
            node *r = new node;
            r->op_type = node::op_kleene_plus;
            r->node_vec.push_back(p);
            return r;
        }

        static node *make_one_or_zero(node *p){
            node *r = new node;
            r->op_type = node::op_one_or_zero;
            r->node_vec.push_back(p);
            return r;
        }

        static node *make_seq(node *p){
            node *r = new node;
            r->op_type = node::op_seq;
            r->node_vec.push_back(p);
            return r;
        }

        static node *make_seq(node *p, node *q){
            p->node_vec.push_back(q);
            return p;
        }

        static node *make_class_range(node *p){
            return make_seq(p);
        }

        static node *make_class_range(node *p, node *q){
            return make_seq(p, q);
        }

        static node *make_range(node *p, node *q){
            node *r = new node;
            r->op_type = node::op_range;
            r->node_vec.push_back(p);
            r->node_vec.push_back(q);
            return r;
        }

        static node *make_charactor(node *p){
            return p;
        }

        static node *make_dot(){
            return nullptr;
        }

        static node *make_class(node *seq){
            node *r = new node;
            r->op_type = node::op_class;
            r->node_vec.push_back(seq);
            return r;
        }

        void downcast(node *&x, node *y){
            x = y;
        }

        void upcast(node *&x, node *y){
            x = y;
        }

        void syntax_error(){
            throw;
        }

        void stack_overflow(){
            throw;
        }
    };

    template<class Iter>
    std::vector<token> tokenize(Iter first, Iter last){
        std::vector<token> seq;
        bool escape = false;
        for(; first != last; ++first){
            token t;
            char c = *first;
            if(escape){
                t.type = regex_parser::token_charactor;
                switch(c){
                case '0':
                    t.c = '\0';
                    break;

                case 'a':
                    t.c = '\a';
                    break;

                case 'b':
                    t.c = '\b';
                    break;

                case 't':
                    t.c = '\t';
                    break;

                case 'n':
                    t.c = '\n';
                    break;

                case 'v':
                    t.c = '\v';
                    break;

                case 'f':
                    t.c = '\f';
                    break;

                case 'r':
                    t.c = '\r';
                    break;

                default:
                    t.c = c;
                    break;
                }
                escape = false;
                seq.push_back(t);
            }else{
                if(c == '\\'){
                    escape = true;
                    continue;
                }
                t.c = c;
                switch(c){
                case '|':
                    t.type = regex_parser::token_vertical_bar;
                    break;

                case '*':
                    t.type = regex_parser::token_asterisk;
                    break;

                case '+':
                    t.type = regex_parser::token_plus;
                    break;

                case '?':
                    t.type = regex_parser::token_question;
                    break;

                case '[':
                    t.type = regex_parser::token_l_bracket;
                    break;

                case ']':
                    t.type = regex_parser::token_r_bracket;
                    break;

                case '(':
                    t.type = regex_parser::token_l_paren;
                    break;

                case ')':
                    t.type = regex_parser::token_r_paren;
                    break;

                case '-':
                    t.type = regex_parser::token_hyphen;
                    break;

                default:
                    t.type = regex_parser::token_charactor;
                    break;
                }
                seq.push_back(t);
            }
        }
        return seq;
    }
} // namespace regex_parser


namespace automaton{
    node::node(const node &other) : edge(other.edge), token_name(new std::string(*other.token_name)), action(new std::string(*other.action)){}
    node::node(node &&other) : edge(std::move(other.edge)), token_name(std::move(other.token_name)), action(std::move(other.action)){}

    std::set<std::size_t> edge(const node_pool &pool, std::size_t s, char c){
        std::set<std::size_t> ret;
        for(auto &i : pool[s].edge){
            if(i.first == c){
                ret.insert(i.second);
            }
        }
        return ret;
    }

    std::set<std::size_t> closure(const node_pool &pool, std::set<std::size_t> T){
        std::set<std::size_t> U;
        do{
            U = T;
            std::set<std::size_t> V;
            for(auto &s : U){
                std::set<std::size_t> W = edge(pool, s, '\0');
                V.insert(W.begin(), W.end());
            }
            for(auto &i : U){
                T.insert(i);
            }
            for(auto &i : V){
                T.insert(i);
            }
        }while(T != U);
        return T;
    }

    std::set<std::size_t> DFA_edge(const node_pool &pool, const std::set<std::size_t> &d, char c){
        std::set<std::size_t> e;
        for(auto &s : d){
            std::set<std::size_t> f = edge(pool, s, c);
            e.insert(f.begin(), f.end());
        }
        return closure(pool, e);
    }

    error_ambiguous_nonterminal_token::error_ambiguous_nonterminal_token(const std::string &str) : std::runtime_error(str){}

    std::set<char> collect_char(const node_pool &pool){
        std::set<char> s;
        for(auto &i : pool){
            for(auto &k : i.edge){
                s.insert(k.first);
            }
        }
        return s;
    }

    node_pool NFA_to_DFA(const node_pool &pool){
        node_pool trans;
        {
            std::set<char> sigma = collect_char(pool);
            std::vector<std::set<std::size_t>> states = { {}, { closure(pool, { 0 }) } };
            std::size_t p = 1, j = 0;
            while(j <= p){
                for(char c : sigma){
                    if(c == '\0'){
                        continue;
                    }
                    std::set<std::size_t> e = DFA_edge(pool, states[j], c);
                    bool find = false;
                    std::size_t i;
                    for(i = 0; i <= p; ++i){
                        if(e == states[i]){
                            find = true;
                            break;
                        }
                    }

                    auto check_ender_tokens = [&](std::size_t i){
                        if(i > 0){
                            trans[j].edge.push_back(std::make_pair(c, i));
                        }
                        for(std::size_t n : states[j]){
                            if(pool[n].token_name){
                                trans[j].token_name.reset(new std::string(*pool[n].token_name));
                                trans[j].action.reset(new std::string(*pool[n].action));
                                break;
                            }
                        }
                    };

                    if(find){
                        if(trans.size() <= j){
                            trans.resize(j + 1);
                        }
                        check_ender_tokens(i);
                    }else{
                        ++p;
                        if(states.size() <= p){
                            states.resize(p + 1);
                        }
                        states[p] = e;
                        if(trans.size() <= j){
                            trans.resize(j + 1);
                        }
                        check_ender_tokens(p);
                    }
                }
                ++j;
            }
        }
        return trans;
    }

    lexer::parsing_error::parsing_error(const std::string &what) : std::runtime_error(what){}

    void lexer::add_rule(const std::string &str, const std::string &token_name, const std::string &action){
        token_info_vector.push_back(token_info{ token_name, action });

        std::vector<regex_parser::token> seq = regex_parser::tokenize(str.begin(), str.end());

        regex_parser::semantic_action sa;
        regex_parser::Parser<regex_parser::node*, regex_parser::semantic_action> parser(sa);

        for(auto &i : seq){
            regex_parser::node *p = nullptr;
            if(i.type == regex_parser::token_charactor){
                p = new regex_parser::node;
                p->c = i.c;
                p->op_type = regex_parser::node::op_charactor;
            }
            parser.post(static_cast<regex_parser::Token>(i.type), p);
        }
        parser.post(regex_parser::token_eof, nullptr);

        std::unique_ptr<regex_parser::node> root;
        {
            regex_parser::node *root_;
            if(!parser.accept(root_)){
                throw parsing_error("parsing error (regular expression) : " + str);
            }
            root.reset(root_);
        }

        std::size_t end;
        if(node_pool.empty()){
            node_pool.push_back({});
            end = root->to_NFA(0, node_pool);
        }else{
            std::size_t start = node_pool.size();
            node_pool[0].edge.push_back(std::make_pair('\0', start));
            node_pool.resize(start + 1);
            end = root->to_NFA(start, node_pool);
        }
        node_pool[end].token_name.reset(new std::string(token_name));
        node_pool[end].action.reset(new std::string(action));
    }

    void lexer::build(){
        node_pool = automaton::NFA_to_DFA(node_pool);
        optimize();
    }

    void lexer::generate_cpp(std::ostream &ofile, const std::string &lexer_namespace){
        indent_type indent;

        // include guard.
        std::string include_guard = lexer_namespace;
        std::transform(include_guard.begin(), include_guard.end(), include_guard.begin(), std::toupper);
        include_guard += "_HPP_";
        ofile << indent() << "#ifndef " << include_guard << "\n";
        ofile << indent() << "#define " << include_guard << "\n\n";

        // include vector.
        ofile << indent() << "#include <memory>\n";
        ofile << indent() << "#include <vector>\n";
        ofile << indent() << "#include <exception>\n";
        ofile << indent() << "#include \"lxq.hpp\"\n\n";

        // enum class.
        ofile << indent() << "namespace lxq{\n";
        ++indent;
        ofile << indent() << "enum class token_id : int{\n";
        ++indent;
        for(std::size_t i = 0; i < token_info_vector.size(); ++i){
            ofile << indent() << token_info_vector[i].name;
            ofile << " = " << token_info_vector[i].identifier << ",\n";
        }
        ofile << indent() << "end = " << (std::numeric_limits<int>::max)() << "\n";
        --indent;
        ofile << indent() << "};\n";
        --indent;
        ofile << indent() << "}\n\n";

        ofile << indent() << "template<class Iter>\n";
        ofile << indent() << "struct " << lexer_namespace << "{\n";
        ++indent;

        // token_type
        ofile << indent() << "struct token_type{\n";
        ++indent;
        ofile << indent() << "using identifier_type = lxq::token_id;\n";
        ofile << indent() << "token_type() : value(nullptr){}\n";
        ofile << indent() << "token_type(const token_type&) = delete;\n";
        ofile << indent() << "token_type(token_type &&other) : first(std::move(other.first)), last(std::move(other.last)), line_num(other.line_num), char_num(other.line_num), word_num(other.word_num), identifier(other.identifier), value(std::move(other.value)){}\n";
        ofile << indent() << "~token_type() = default;\n";
        ofile << indent() << "Iter first, last;\n";
        ofile << indent() << "std::size_t line_num, char_num, word_num;\n";
        ofile << indent() << "identifier_type identifier;\n";
        ofile << indent() << "std::unique_ptr<lxq::semantic_data> value;\n";
        --indent;
        ofile << indent() << "};\n\n";

        // tokenize function.
        ofile << indent() << "template<class Action>\n";
        ofile << indent() << "static std::vector<token_type> tokenize(Iter iter, Iter end, Action &action){\n";
        ++indent;
        ofile << indent() << "std::vector<token_type> result;\n";
        ofile << indent() << "Iter first = iter;\n";
        ofile << indent() << "std::size_t line_num = 0, char_num = 0, word_num = 0;\n";
        ofile << indent() << "char c;\n\n";

        for(std::size_t i = 1; i < node_pool.size(); ++i){
            if(unused_node_set.find(i) != unused_node_set.end()){
                continue;
            }

            ofile << indent() << "state_" << i << ":;\n";
            if(i == 1){
                ofile << indent() << "if(iter == end){\n";
                ++indent;
                ofile << indent() << "goto end_of_tokenize;\n";
                --indent;
                ofile << indent() << "}\n";
            }else if(node_pool[i].token_name){
                ofile << indent() << "if(iter == end){\n";
                ++indent;
                ofile << indent() << "token_type t;\n";
                ofile << indent() << "t.first = first;\n";
                ofile << indent() << "t.last = iter;\n";
                ofile << indent() << "t.line_num = line_num;\n";
                ofile << indent() << "t.char_num = char_num;\n";
                ofile << indent() << "t.word_num = word_num++;\n";
                ofile << indent() << "t.identifier = token_type::identifier_type::" << *node_pool[i].token_name << ";\n";
                if(node_pool[i].action->size() > 0){
                    ofile << indent() << "t.value = std::move(std::unique_ptr<lxq::semantic_data>(action." << *node_pool[i].action << "(first, iter)));\n";
                }
                ofile << indent() << "result.push_back(std::move(t));\n";
                ofile << indent() << "goto end_of_tokenize;\n";
                --indent;
                ofile << indent() << "}\n";
            }else{
                ofile << indent() << "if(iter == end){\n";
                ++indent;
                ofile << indent() << "throw std::runtime_error(\"lexical error : state " << i << "\");\n";
                --indent;
                ofile << indent() << "}\n";
            }

            if(node_pool[i].edge.size() > 0){
                ofile << indent() << "c = *iter;\n";

                std::map<std::size_t, std::vector<std::size_t>> edge_inv_map;
                for(auto &j : node_pool[i].edge){
                    edge_inv_map[j.second].push_back(j.first);
                }
                ofile << indent() << "switch(c){\n";
                for(auto &j : edge_inv_map){
                    bool nline = false;
                    for(std::size_t k = 0; k < j.second.size(); ++k){
                        if(j.second[k] == '\n'){
                            nline = true;
                        }
                        std::string str = std::to_string(j.second[k]);
                        ofile << (k % 8 == 0 ? indent() : "") << "case " << [](std::size_t n){ std::string s; for(std::size_t i = 0; i < 3 - n; ++i){ s += " "; } return s; }(str.size()) << str << ((k + 1) % 8 == 0 ? ":\n" : ": ");
                    }
                    if((j.second.size()) % 8 != 0){
                        ofile << "\n";
                    }
                    ++indent;
                    if(nline){
                        if(j.second.size() > 1){
                            ofile << indent() << "if(c == " << static_cast<int>('\n') << "){\n";
                            ++indent;
                        }

                        ofile << indent() << "char_num = 0;\n";
                        ofile << indent() << "word_num = 0;\n";
                        ofile << indent() << "++line_num;\n";

                        if(j.second.size() > 1){
                            --indent;
                            ofile << indent() << "}\n";
                        }
                    }
                    ofile << indent() << "++char_num;\n";
                    ofile << indent() << "++iter;\n";
                    ofile << indent() << "goto state_" << j.first << ";\n";
                    --indent;
                }
                ofile << indent() << "}\n";
            }

            if(i == 1){
                ofile << indent() << "throw std::runtime_error(\"lexical error : state 1\");\n\n";
            }else if(node_pool[i].token_name){
                ofile << indent() << "{\n";
                ++indent;
                ofile << indent() << "token_type t;\n";
                ofile << indent() << "t.first = first;\n";
                ofile << indent() << "t.last = iter;\n";
                ofile << indent() << "t.line_num = line_num;\n";
                ofile << indent() << "t.char_num = char_num;\n";
                ofile << indent() << "t.word_num = word_num++;\n";
                ofile << indent() << "t.identifier = token_type::identifier_type::" << *node_pool[i].token_name << ";\n";
                if(node_pool[i].action->size() > 0){
                    ofile << indent() << "t.value = std::move(std::unique_ptr<lxq::semantic_data>(action." << *node_pool[i].action << "(first, iter)));\n";
                }
                ofile << indent() << "result.push_back(std::move(t));\n";
                ofile << indent() << "first = iter;\n";
                ofile << indent() << "goto state_1;\n";
                --indent;
                ofile << indent() << "}\n\n";
            }else{
                ofile << indent() << "throw std::runtime_error(\"lexical error : state " << i << "\");\n\n";
            }
        }

        ofile << indent() << "end_of_tokenize:;\n";
        ofile << indent() << "{\n";
        ++indent;
        ofile << indent() << "token_type t;\n";
        ofile << indent() << "t.first = iter;\n";
        ofile << indent() << "t.last = iter;\n";
        ofile << indent() << "t.line_num = 0;\n";
        ofile << indent() << "t.char_num = 0;\n";
        ofile << indent() << "t.word_num = 0;\n";
        ofile << indent() << "t.identifier = token_type::identifier_type::end;\n";
        ofile << indent() << "result.push_back(std::move(t));\n";
        --indent;
        ofile << indent() << "}\n";
        ofile << indent() << "return result;\n";
        --indent;
        ofile << indent() << "}\n";
        --indent;
        ofile << indent() << "};\n";

        // end of include guard.
        ofile << indent() << "#endif // " << include_guard << "\n\n";
    }

    void lexer::optimize(){
        using edge = std::set<std::pair<char, std::size_t>>;
        using node = std::set<edge>;

        std::set<std::pair<std::size_t, std::size_t>> inequality_pair_set;
        bool mod;
        do{
            mod = false;
            for(std::size_t i = 0; i < node_pool.size(); ++i){
                node node_i;
                edge edge_i;
                for(auto &a : node_pool[i].edge){
                    edge_i.insert(a);
                }
                node_i.insert(std::move(edge_i));

                for(std::size_t j = i + 1; j < node_pool.size(); ++j){
                    if(
                        node_pool[i].token_name && !node_pool[j].token_name ||
                        !node_pool[i].token_name && node_pool[j].token_name ||
                        node_pool[i].token_name != node_pool[j].token_name
                        ){
                        mod = inequality_pair_set.insert(std::make_pair(i, j)).second || mod;
                    }else{
                        node node_j;
                        edge edge_j;
                        for(auto &a : node_pool[j].edge){
                            edge_j.insert(a);
                        }
                        node_j.insert(std::move(edge_j));

                        if(node_i != node_j){
                            mod = inequality_pair_set.insert(std::make_pair(i, j)).second || mod;
                        }
                    }
                }
            }
        }while(mod);

        std::map<std::size_t, std::size_t> equality_pair_map;
        for(std::size_t i = 0; i < node_pool.size(); ++i){
            for(std::size_t j = i + 1; j < node_pool.size(); ++j){
                if(inequality_pair_set.find(std::make_pair(i, j)) != inequality_pair_set.end()){
                    continue;
                }else{
                    equality_pair_map.insert(std::make_pair(j, i));
                }
            }
        }

        for(auto &a : node_pool){
            for(auto &e : a.edge){
                auto iter = equality_pair_map.find(e.second);
                if(iter != equality_pair_map.end()){
                    e.second = equality_pair_map[e.second];
                    unused_node_set.insert(e.second);
                }
            }
        }
    }
}
