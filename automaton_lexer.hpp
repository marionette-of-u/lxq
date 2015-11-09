#ifndef AUTOMATON_LEXER_HPP_
#define AUTOMATON_LEXER_HPP_

#include <initializer_list>
#include <functional>
#include <exception>
#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <stack>
#include <set>
#include <map>
#include <cctype>
#include <cstdio>
#include "regexp_parser.hpp"

namespace automaton{
    class node{
    public:
        enum{
            eos = 128
        };

        node() = default;
        node(const node &other);
        node(node &&other);
        ~node() = default;

        std::vector<std::pair<char, std::size_t>> edge;
        std::unique_ptr<std::string> token_name, action;
    };

    using node_pool = std::vector<node>;

    std::set<std::size_t> edge(const node_pool &pool, std::size_t s, char c);
    std::set<std::size_t> closure(const node_pool &pool, std::set<std::size_t> T);
    std::set<std::size_t> DFA_edge(const node_pool &pool, const std::set<std::size_t> &d, char c);

    struct error_ambiguous_nonterminal_token : public std::runtime_error{
        error_ambiguous_nonterminal_token(const std::string &str);
    };

    std::set<char> collect_char(const node_pool &pool);
    node_pool NFA_to_DFA(const node_pool &pool);

    class lexer{
    public:
        struct parsing_error : public std::runtime_error{
            parsing_error(const std::string &what);
            ~parsing_error() = default;
        };

        struct token_info{
            std::string name;
            std::string action;
            std::size_t identifier;
        };

        using token_name_vector_type = std::vector<token_info>;
        token_name_vector_type token_info_vector;

        lexer() = default;
        virtual ~lexer() = default;
        void add_rule(const std::string &str, const std::string &token_name, const std::string &action);
        void build();
        void generate_cpp(std::ostream &ofile_path, const std::string &lexer_namespace);

    private:
        void optimize();

        node_pool node_pool;
        std::set<std::size_t> unused_node_set;
    };
}

#endif // AUTOMATON_LEXER_HPP_
