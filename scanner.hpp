#ifndef SCANNER_HPP_
#define SCANNER_HPP_

#include <set>
#include <string>
#include <iostream>
#include <limits>
#include <iterator>
#include <regex>
#include <memory>
#include <fstream>
#include <list>
#include <sstream>
#include <tuple>
#include <cstdio>
#include <cctype>
#include "lalr.hpp"
#include "automaton_lexer.hpp"
#include "common.hpp"

namespace scanner{
    template<class ValueType, class TermType>
    struct basic_token{
        using value_type = ValueType;
        using term_type = TermType;
        value_type value;
        term_type term = term_type();
        std::size_t char_num = 0, word_num = 0, line_num = 0;

        basic_token() = default;

        basic_token &operator =(basic_token const &other){
            value = other.value;
            term = other.term;
            char_num = other.char_num;
            word_num = other.word_num;
            line_num = other.line_num;
            return *this;
        }
        
        bool operator <(basic_token const &other) const{
            return value < other.value;
        }

        basic_token(basic_token const &other) :
            value(other.value), term(other.term),
            char_num(other.char_num), line_num(other.line_num)
        {}

        basic_token(basic_token &&other) :
            value(std::move(other.value)), term(std::move(other.term)),
            char_num(other.char_num), line_num(other.line_num)
        {}

        ~basic_token() = default;

        static basic_token const &dummy_storage(){
            static basic_token storage;
            return storage;
        }
    };

    struct scanning_exception{
        std::string message;
        std::size_t char_num = 0, word_num = 0, line_num = 0;
        scanning_exception() = default;
        scanning_exception(const std::string &message, std::size_t char_num, std::size_t word_num, std::size_t line_num);
        ~scanning_exception() = default;
    };

    using scanning_exception_seq = std::vector<scanning_exception>;

    using term_type = int;

    extern symbol_manager_type<std::string> symbol_manager;

    struct eos_functor{
        inline term_type operator ()() const{
            return std::numeric_limits<int>::max();
        }
    };

    struct dummy_functor{
        inline term_type operator ()() const{
            return std::numeric_limits<term_type>::max() - 1;
        }
    };

    struct whitespace_functor{
        inline term_type operator ()() const{
            return std::numeric_limits<term_type>::max() - 2;
        }
    };

    struct epsilon_functor{
        inline term_type operator ()() const{
            return 0;
        }
    };

    struct is_terminal_symbol{
        template<class G>
        inline bool operator ()(term_type a, G const &) const{
            return a > 0;
        }
    };

    extern term_type const
        value,
        comma,
        dot,
        question,
        exclamation,
        plus,
        hyphen,
        asterisk,
        slash,
        colon,
        semicolon,
        l_square_bracket,
        r_square_bracket,
        l_curly_bracket,
        r_curly_bracket,
        l_bracket,
        r_bracket,
        l_round_paren,
        r_round_paren,
        vertical_bar,
        equal,
        string,
        identifier;

    using vstring = std::vector<char>;
    using vstring_range = string_iter_pair<vstring>;
    using token_type = basic_token<vstring_range, term_type>;

    class lexer{
    private:
        std::map<term_type, std::regex> regex_map_subst;
        std::vector<token_type> token_seq_subst;
        std::size_t char_count = 0, line_count = 0;
        std::size_t tab_width;

        term_type t(std::string const &str);

    public:
        decltype(regex_map_subst) const &regex_map = regex_map_subst;
        decltype(token_seq_subst) const &token_seq = token_seq_subst;

        lexer(std::size_t tab_width = 4) : tab_width(tab_width){}
        void new_regex(const std::string &r, term_type token_kind);
        void clear_token_seq();
        void tokenize(vstring::const_iterator first, vstring::const_iterator last);
    };

    struct rhs_seq_element_type{
        token_type identifier, arg;
        std::size_t display_pos;
        bool operator <(rhs_seq_element_type const &other) const;
    };

    struct lalr_generator_semantic_type{
        vstring_range action;
        std::map<std::size_t, rhs_seq_element_type const*> const *arg_to_element;
    };

    class lalr_generator_type : public lalr<lalr_generator_semantic_type, term_type, eos_functor, dummy_functor, is_terminal_symbol, epsilon_functor>{
    public:
        using lalr<semantic_type, term_type, eos_functor, dummy_functor, is_terminal_symbol, epsilon_functor>::lalr;
        using symbol_to_term_sequence_map = std::map<term, term_sequence>;
        using exception_seq = std::vector<std::runtime_error>;
        symbol_data_map symbol_data_map;
        grammar grammar;
        symbol_manager_type<vstring_range> symbol_manager;
        symbol_to_term_sequence_map symbol_to_term_sequence;
    };

    term_type nt(std::string const &str);

    class ast{
    public:
        token_type token;
        std::vector<ast*> nodes;

        ast() = default;
        ast(token_type const &token, std::vector<ast*> const &nodes);
        virtual ~ast();

        inline static ast const &dummy_storage(){
            static ast storage(token_type::dummy_storage(), std::vector<ast*>());
            return storage;
        }

        inline static ast const *dummy_storage_ptr(){
            return &(dummy_storage());
        }
    };

    class scanning_data_type{
    public:
        enum class linkdir{
            nonassoc,
            left,
            right
        };

        std::vector<ast*> ast_stack;
        ast const
            *regexp_namespace,
            *regexp_body,
            *token_namespace,
            *token_body,
            *grammar_namespace,
            *expr_statements;
        std::size_t current_regexp_token_priority = 0;
        std::size_t current_token_priority = (std::numeric_limits<std::size_t>::max)();
        std::size_t current_scanning_rhs_token_count = 0;

        struct regexp_symbol_data_type{
            ast const *action;
            std::size_t priority;
            vstring_range regexp;
        };

        using regexp_symbol_data_map_type = std::map<token_type, regexp_symbol_data_type>;
        regexp_symbol_data_map_type regexp_symbol_data_map;

        struct symbol_data_type{
            std::size_t priority;
            linkdir dir;
        };

        using symbol_data_map_type = std::map<token_type, symbol_data_type>;
        symbol_data_map_type symbol_data_map;
        std::vector<symbol_data_map_type::const_iterator> ordered_token_iters;
        scanning_exception_seq identifier_seq_error;

        class rhs_seq : public std::vector<rhs_seq_element_type>{
        public:
            using base = std::vector<rhs_seq_element_type>;
            using base::base;
            void make_arg_to_element_map() const;
            token_type semantic_action, tag;
            std::map<std::size_t, rhs_seq_element_type const*> mutable arg_to_element;
            std::size_t mutable max_arg;
        };

        using rhs_seq_set = std::set<rhs_seq>;
        std::map<token_type, rhs_seq_set> rules;
        scanning_exception_seq expr_statements_error;
        lalr_generator_type::states states_prime, states;
        automaton::lexer automaton_lexer;
        lalr_generator_type lalr_generator;
        lalr_generator_type::make_result lalr_generator_make_result;

        ~scanning_data_type();

        void collect_info();
        void generate_cpp_semantic_data(std::ostream &os);
        void generate_cpp(std::ostream &os);

    private:
        ast const *get_arg_opt(ast const *ptr);
        ast const *get_semantic_action(ast const *ptr);
        ast const *get_tag_opt(ast const *ptr);
        void get_rhs_seq(ast const *ptr, rhs_seq &seq);
        rhs_seq get_rhs_seq_opt(ast const *ptr);
        void get_rhs(ast const *ptr, rhs_seq_set &set);
        void get_expr(ast const *ptr);
        void get_expr_statements(ast const *ptr);
        void get_top_level_seq_statements(ast const *ptr);
        void get_top_level_seq_statements_element(ast const *ptr);
        void get_block_with_link_dir(ast const *ptr);
        linkdir get_link_dir(ast const *ptr);
        void get_seq_statements(ast const *ptr, linkdir dir);
        void get_seq_statements_element(ast const *ptr, linkdir dir);
        void get_identifier_seq(ast const *ptr, linkdir dir);
        void get_regexp_statements(ast const *ptr);
        void get_regexp_seq(ast const *ptr);
    };

    extern scanning_data_type scanning_data;

    using arg_type = std::vector<token_type>;
    using semantic_type = std::function<token_type(term_type, arg_type const&, scanning_data_type&)>;

    class scanner : public lalr<
        semantic_type,
        term_type,
        eos_functor,
        dummy_functor,
        is_terminal_symbol,
        epsilon_functor
    >{
    public:
        template<class InputIter>
        bool parse(make_result const &table, InputIter first, InputIter last){
            std::vector<std::size_t> state_stack;
            std::vector<token_type> value_stack;
            state_stack.push_back(table.first);
            while(true){
                token_type const &value = *first;
                term const &t = value.term;
                std::size_t s = state_stack.back();
                auto const &table_second(table.parsing_table.find(s)->second);
                auto iter = table_second.find(t);
                if(iter == table_second.end()){
                    throw scanning_exception("parsing error.", value.char_num, value.word_num, value.line_num);
                }
                lr_parsing_table_item const &i = table.parsing_table.find(s)->second.find(t)->second;
                if(i.action == lr_parsing_table_item::enum_action::shift){
                    state_stack.push_back(i.num);
                    value_stack.push_back(value);
                    ast *a = new ast;
                    a->token = value;
                    scanning_data.ast_stack.push_back(a);
                    ++first;
                }else if(i.action == lr_parsing_table_item::enum_action::reduce){
                    auto &p(*table.n2r.find(i.num));
                    std::size_t norm = p.second.second->size();
                    state_stack.resize(state_stack.size() - norm);
                    if(state_stack.empty()){
                        throw scanning_exception("parsing error.", value.char_num, value.word_num, value.line_num);
                    }
                    state_stack.push_back(table.goto_table.find(state_stack.back())->second.find(p.second.first)->second);
                    decltype(value_stack) arg(value_stack.begin() + (value_stack.size() - norm), value_stack.end());
                    value_stack.resize(value_stack.size() - norm);
                    value_stack.push_back(p.second.second->semantic_data(p.second.first, arg, scanning_data));
                }else if(i.action == lr_parsing_table_item::enum_action::accept){
                    auto &p(*table.n2r.find(i.num));
                    std::size_t norm = p.second.second->size();
                    state_stack.resize(state_stack.size() - norm);
                    if(state_stack.empty()){
                        throw scanning_exception("parsing error.", value.char_num, value.word_num, value.line_num);
                    }
                    decltype(value_stack) arg(value_stack.begin() + (value_stack.size() - norm), value_stack.end());
                    value_stack.resize(value_stack.size() - norm);
                    value_stack.push_back(p.second.second->semantic_data(p.second.first, arg, scanning_data));
                    break;
                }
            }
            return first == last;
        }
    };

    void init_lexer(lexer &lex);
    void init_grammar(scanner::grammar &grammar);
    void scan(const std::string ifile_path, std::string out_path);
} // namespace scanner

#endif // SCANNER_HPP_
