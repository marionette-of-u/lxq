#include "automaton_lexer.hpp"
#include "common.hpp"

namespace regexp_parser{
    struct token{
        char c;
        int type;
    };

    class regexp_ast{
    public:
        regexp_ast() = default;
        regexp_ast(const char c) : c(c){}
        regexp_ast(const regexp_ast &other) : c(other.c){
            for(auto &iter : other.node_vec){
                node_vec.push_back(iter->clone());
            }
        }

        regexp_ast(regexp_ast &&other) : c(std::move(other.c)), node_vec(std::move(other.node_vec)){}

        virtual ~regexp_ast(){
            for(auto &iter : node_vec){
                delete iter;
            }
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const{
            assert(false);
            return (std::numeric_limits<std::size_t>::max)();
        }

        virtual regexp_ast *clone() const = 0;
        template<class Derived>
        regexp_ast *clone_impl() const{
            regexp_ast *ptr = new Derived;
            ptr->c = c;
            for(auto &iter : node_vec){
                ptr->node_vec.push_back(iter->clone());
            }
            return ptr;
        }

        char c;
        std::vector<regexp_ast*> node_vec;
    };

    class regexp_union : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_union>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            std::size_t e1 = node_vec[0]->to_NFA(start, pool);
            std::size_t e2 = node_vec[1]->to_NFA(start, pool);
            pool.push_back(automaton::node());
            r = pool.size() - 1;
            pool[e1].edge.push_back(std::make_pair('\0', r));
            pool[e2].edge.push_back(std::make_pair('\0', r));
            return r;
        }
    };

    class regexp_concat : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_concat>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            r = node_vec[0]->to_NFA(start, pool);
            r = node_vec[1]->to_NFA(r, pool);
            return r;
        }
    };

    class regexp_kleene : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_kleene>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            r = node_vec[0]->to_NFA(start, pool);
            pool[start].edge.push_back(std::make_pair('\0', r));
            pool[r].edge.push_back(std::make_pair('\0', start));
            return r;
        }
    };

    class regexp_kleene_plus : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_kleene_plus>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            std::size_t q = node_vec[0]->to_NFA(start, pool);
            r = node_vec[0]->to_NFA(q, pool);
            pool[q].edge.push_back(std::make_pair('\0', r));
            pool[r].edge.push_back(std::make_pair('\0', q));
            return r;
        }
    };

    class regexp_one_or_zero : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_one_or_zero>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            r = pool.size() - 1;
            std::size_t n = node_vec[0]->to_NFA(start, pool);
            pool[n].edge.push_back(std::make_pair('\0', r));
            pool[start].edge.push_back(std::make_pair('\0', r));
            return r;
        }
    };

    class regexp_range : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_range>();
        }
    };

    class regexp_char : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_char>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            pool[start].edge.push_back(std::make_pair(c, pool.size() - 1));
            r = pool.size() - 1;
            return r;
        }
    };

    class regexp_set : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_set>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            r = pool.size() - 1;
            for(std::size_t i = 0; i < node_vec.size(); ++i){
                for(std::size_t j = 0; j < node_vec[i]->node_vec.size(); ++j){
                    if(dynamic_cast<regexp_char*>(node_vec[i]->node_vec[j])){
                        pool[start].edge.push_back(std::make_pair(node_vec[i]->node_vec[j]->c, r));
                    }else if(dynamic_cast<regexp_range*>(node_vec[i]->node_vec[j])){
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
            return r;
        }
    };

    class regexp_neagtive_set : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_neagtive_set>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            std::set<char> set;
            for(int i = -127; i < 128; ++i){
                set.insert(static_cast<char>(i));
            }
            pool.push_back(automaton::node());
            r = pool.size() - 1;
            for(std::size_t i = 0; i < node_vec.size(); ++i){
                for(std::size_t j = 0; j < node_vec[i]->node_vec.size(); ++j){
                    if(dynamic_cast<regexp_char*>(node_vec[i]->node_vec[j])){
                        set.erase(node_vec[i]->node_vec[j]->c);
                    }else if(dynamic_cast<regexp_range*>(node_vec[i]->node_vec[j])){
                        if(node_vec[i]->node_vec[j]->node_vec[0]->c > node_vec[i]->node_vec[j]->node_vec[1]->c){
                            std::swap(node_vec[i]->node_vec[j]->node_vec[0]->c, node_vec[i]->node_vec[j]->node_vec[1]->c);
                        }
                        for(int c = node_vec[i]->node_vec[j]->node_vec[0]->c; c <= node_vec[i]->node_vec[j]->node_vec[1]->c; ++c){
                            set.erase(c);
                        }
                    }
                }
            }
            for(char c : set){
                if(c == 0){ continue; }
                pool[start].edge.push_back({});
                pool[start].edge.back().first = c;
                pool[start].edge.back().second = r;
            }
            return r;
        }
    };

    class regexp_set_item : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_set_item>();
        }
    };

    class regexp_eos : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_eos>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            pool[start].edge.push_back(std::make_pair(automaton::node::eos, pool.size() - 1));
            r = pool.size() - 1;
            return r;
        }
    };

    class regexp_after_nline : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_after_nline>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            pool[start].edge.push_back(std::make_pair('\n', pool.size() - 1));
            r = pool.size() - 1;
            return node_vec[0]->to_NFA(r, pool);
        }
    };

    class regexp_any : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_any>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r;
            pool.push_back(automaton::node());
            r = pool.size() - 1;
            for(int i = -127; i <= 128; ++i){
                if(i == 0){
                    continue;
                }
                pool[start].edge.push_back(std::make_pair(i, r));
            }
            return r;
        }
    };

    class regexp_str : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_str>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r = start;
            for(auto &iter : node_vec){
                r = iter->to_NFA(r, pool);
            }
            return r;
        }
    };

    class regexp_char_seq : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_char_seq>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r = start;
            for(auto &iter : node_vec){
                r = iter->to_NFA(r, pool);
            }
            return r;
        }
    };

    class regexp_n_to_m : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_n_to_m>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t rr = start;
            for(std::size_t i = 0; i < n; ++i){
                rr = node_vec[0]->to_NFA(rr, pool);
            }
            pool.push_back(automaton::node());
            std::size_t r = pool.size() - 1;
            for(std::size_t i = 0; i < m - n; ++i){
                rr = node_vec[0]->to_NFA(rr, pool);
                pool[rr].edge.push_back(std::make_pair('\0', r));
            }
            return r;
        }

        std::size_t n, m;
    };

    class regexp_n : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_n>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r = start;
            for(std::size_t i = 0; i < n; ++i){
                r = node_vec[0]->to_NFA(r, pool);
            }
            return r;
        }

        std::size_t n;
    };

    class regexp_m : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_m>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            std::size_t r = start;
            for(std::size_t i = 0; i < m; ++i){
                r = node_vec[0]->to_NFA(r, pool);
            }
            std::size_t s = node_vec[0]->to_NFA(r, pool);
            pool[s].edge.push_back(std::make_pair('\0', r));
            return r;
        }

        std::size_t m;
    };

    class regexp_identity : public regexp_ast{
    public:
        virtual regexp_ast *clone() const override{
            return clone_impl<regexp_identity>();
        }

        virtual std::size_t to_NFA(std::size_t start, automaton::node_pool &pool) const override{
            return node_vec[0]->to_NFA(start, pool);
        }
    };

    struct semantic_action{
        regexp_ast *make_union(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete c;
            regexp_union *ptr = new regexp_union;
            ptr->node_vec.push_back(a);
            ptr->node_vec.push_back(b);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_concat(regexp_ast *a, regexp_ast *b){
            regexp_concat *ptr = new regexp_concat;
            ptr->node_vec.push_back(a);
            ptr->node_vec.push_back(b);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_kleene(regexp_ast *a, regexp_ast *b){
            delete b;
            regexp_kleene *ptr = new regexp_kleene;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_kleene_plus(regexp_ast *a, regexp_ast *b){
            delete b;
            regexp_kleene_plus *ptr = new regexp_kleene_plus;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_one_or_zero(regexp_ast *a, regexp_ast *b){
            delete b;
            regexp_one_or_zero *ptr = new regexp_one_or_zero;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_n_to_m(regexp_ast *a, regexp_ast *b, regexp_ast *c, regexp_ast *d, regexp_ast *e){
            delete d;
            delete e;
            regexp_ast *r = nullptr;
            auto to_num= [](regexp_ast *ptr){
                std::string str;
                str += ptr->c;
                for(auto &iter : ptr->node_vec){
                    str += iter->c;
                }
                return std::atoi(str.c_str());
            };
            if(!c){
                regexp_n *ptr = new regexp_n;
                ptr->node_vec.push_back(a);
                ptr->n = to_num(b);
                r = ptr;
            }else if(c->c == ','){
                delete c;
                regexp_m *ptr = new regexp_m;
                ptr->node_vec.push_back(a);
                ptr->m = to_num(b);
                r = ptr;
            }else{
                regexp_n_to_m *ptr = new regexp_n_to_m;
                ptr->node_vec.push_back(a);
                ptr->n = to_num(b);
                ptr->m = to_num(c);
                r = ptr;
            }
            if(!r){ throw std::runtime_error("illegal form to 'r{n, m}'"); }
            return r;
        }

        regexp_ast *make_m(regexp_ast *a, regexp_ast *b){
            if(a){
                delete b;
                return a;
            }else{
                return b;
            }
        }

        regexp_ast *make_elementary_regexp(regexp_ast *a){
            ast = a;
            return a;
        }

        regexp_ast *make_group(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete b;
            delete c;
            ast = a;
            return a;
        }

        regexp_ast *make_str(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete b;
            delete c;
            regexp_str *ptr = new regexp_str;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_char_seq(regexp_ast *a, regexp_ast *b){
            a->node_vec.push_back(b);
            ast = a;
            return a;
        }

        regexp_ast *make_char_seq(regexp_ast *a){
            ast = a;
            return a;
        }

        regexp_ast *make_any(regexp_ast *a){
            delete a;
            regexp_any *ptr = new regexp_any;
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_after_nline(regexp_ast *a, regexp_ast *b){
            delete b;
            regexp_after_nline *ptr = new regexp_after_nline;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_eos(regexp_ast *a){
            delete a;
            regexp_eos *ptr = new regexp_eos;
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_char(regexp_ast *a){
            regexp_char *ptr = new regexp_char;
            ptr->c = a->c;
            delete a;
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_char(regexp_ast *a, regexp_ast *b){
            delete b;
            return make_char(a);
        }

        regexp_ast *make_set_or_class(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete b;
            delete c;
            return a;
        }

        regexp_ast *make_set_or_class_content(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete b;
            delete c;
            regexp_set_item *ptr = new regexp_set_item;
            ptr->node_vec.push_back(a);
            return ptr;
        }

        regexp_ast *make_class_content(regexp_ast *a, regexp_ast *b, regexp_ast *c){
            delete b;
            delete c;
            return a;
        }

        regexp_ast *make_set_content(regexp_ast *a, regexp_ast *b){
            regexp_ast *ptr;
            if(!b){
                ptr = new regexp_set;
            }else{
                ptr = new regexp_neagtive_set;
            }
            delete b;
            ptr->node_vec.push_back(a);
            ast = ptr;
            return ptr;
        }

        regexp_ast *make_set_item(regexp_ast *a){
            regexp_set_item *ptr = new regexp_set_item;
            ptr->node_vec.push_back(a);
            return ptr;
        }

        regexp_ast *make_set_item(regexp_ast *a, regexp_ast *b){
            a->node_vec.push_back(b->node_vec[0]);
            b->node_vec.clear();
            delete b;
            return a;
        }

        regexp_ast *make_range(regexp_ast *a, regexp_ast *b){
            regexp_range *ptr = new regexp_range;
            ptr->node_vec.push_back(a);
            ptr->node_vec.push_back(b);
            return ptr;
        }

        regexp_ast *identity(regexp_ast *a){
            ast = a;
            return a;
        }

        void downcast(regexp_ast *&x, regexp_ast *y){
            x = y;
        }

        void upcast(regexp_ast *&x, regexp_ast *y){
            x = y;
        }

        void syntax_error(){
            throw std::runtime_error("");
        }

        void stack_overflow(){
            throw std::runtime_error("");
        }

    private:
        regexp_ast *ast;
    };
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
        regexp_parser::semantic_action sa;
        regexp_parser::parser<regexp_parser::regexp_ast*, regexp_parser::semantic_action> parser(sa);
        for(char c : str){
            switch(c){
            case '|':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_or, ptr);
                }
                break;

            case '*':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_star, ptr);
                }
                break;

            case '+':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_plus, ptr);
                }
                break;

            case '?':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_question, ptr);
                }
                break;

            case '(':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_left_pare, ptr);
                }
                break;

            case ')':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_right_pare, ptr);
                }
                break;

            case '{':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_left_brace, ptr);
                }
                break;

            case '}':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_right_brace, ptr);
                }
                break;

            case '.':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_dot, ptr);
                }
                break;

            case '\\':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_backslash, ptr);
                }
                break;

            case '[':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_set_left_bracket, ptr);
                }
                break;

            case '^':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_hat, ptr);
                }
                break;

            case ']':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_set_right_bracket, ptr);
                }
                break;

            case '-':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_minus, ptr);
                }
                break;

            case ',':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_comma, ptr);
                }
                break;

            case ':':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_colon, ptr);
                }
                break;

            case '\'':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_quote, ptr);
                }
                break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_number, ptr);
                }
                break;

            default:
                {
                    regexp_parser::regexp_char *ptr = new regexp_parser::regexp_char;
                    ptr->c = c;
                    parser.post(regexp_parser::regexp_token_symbol_any_non_metacharacter, ptr);
                }
                break;
            }
        }
        parser.post(regexp_parser::regexp_token_0, nullptr);
        regexp_parser::regexp_ast *root;
        if(!parser.accept(root)){
            throw std::runtime_error("regexp error '" + token_name + "'");
        }

        token_info_vector.push_back(token_info{ token_name, action });

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
        ofile << indent() << "error = " << error_token_functor()() << ",\n";
        ofile << indent() << "end = " << eos_functor()() << "\n";
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

            std::map<std::size_t, std::set<int>> edge_inv_map;
            std::map<std::size_t, std::set<int>> other_edge_inv_map;
            for(auto &j : node_pool[i].edge){
                if(j.first >= -128 && j.first <= 127){
                    edge_inv_map[j.second].insert(j.first);
                }else{
                    other_edge_inv_map[j.second].insert(j.first);
                }
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
                if(*node_pool[i].action != "drop"){
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
                }
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
                ofile << indent() << "switch(c){\n";
                for(auto &j : edge_inv_map){
                    bool nline = false;
                    std::size_t k = 0;
                    for(auto kter = j.second.begin(); kter != j.second.end(); ++kter, ++k){
                        if(*kter == '\n'){
                            nline = true;
                        }
                        std::string str = std::to_string(*kter);
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
                if(*node_pool[i].action != "drop"){
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
                }
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
