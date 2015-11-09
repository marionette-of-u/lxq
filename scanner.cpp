#include "scanner.hpp"
#include "common.hpp"

namespace scanner{
    scanning_exception::scanning_exception(const std::string &message, std::size_t char_num, std::size_t word_num, std::size_t line_num)
        : message(message), char_num(char_num), word_num(word_num), line_num(line_num){}

    symbol_manager_type<std::string> symbol_manager;

    term_type const
        value = symbol_manager.set_terminal("value"),
        comma = symbol_manager.set_terminal("comma"),
        dot = symbol_manager.set_terminal("dot"),
        question = symbol_manager.set_terminal("question"),
        exclamation = symbol_manager.set_terminal("exclamation"),
        plus = symbol_manager.set_terminal("plus"),
        hyphen = symbol_manager.set_terminal("hyphen"),
        asterisk = symbol_manager.set_terminal("asterisk"),
        slash = symbol_manager.set_terminal("slash"),
        colon = symbol_manager.set_terminal("colon"),
        semicolon = symbol_manager.set_terminal("semicolon"),
        l_square_bracket = symbol_manager.set_terminal("l_square_bracket"),
        r_square_bracket = symbol_manager.set_terminal("r_square_bracket"),
        l_curly_bracket = symbol_manager.set_terminal("l_curly_bracket"),
        r_curly_bracket = symbol_manager.set_terminal("r_curly_bracket"),
        l_bracket = symbol_manager.set_terminal("l_bracket"),
        r_bracket = symbol_manager.set_terminal("r_bracket"),
        l_round_paren = symbol_manager.set_terminal("l_round_paren"),
        r_round_paren = symbol_manager.set_terminal("r_round_paren"),
        vertical_bar = symbol_manager.set_terminal("vertical_bar"),
        equal = symbol_manager.set_terminal("equal"),
        string = symbol_manager.set_terminal("string"),
        identifier = symbol_manager.set_terminal("identifier");

    term_type lexer::t(std::string const &str){
        return symbol_manager.set_terminal(str);
    }

    void lexer::new_regex(const std::string &r, term_type token_kind){
        std::regex regex("^(" + r + ")" , std::regex_constants::optimize);
        regex_map_subst.insert(std::make_pair(token_kind, regex));
    }

    void lexer::clear_token_seq(){
        token_seq_subst.clear();
        char_count = 0;
        line_count = 0;
    }

    void lexer::tokenize(vstring::const_iterator first, vstring::const_iterator last){
        while(first != last){
            bool match = false;
            vstring::const_iterator longest_first = first, longest_last = first;
            int current_term;
            for(auto &e : regex_map){
                std::match_results<vstring::const_iterator> match_results;
                std::regex_search(first, last, match_results, e.second);
                if(!match_results.empty() && match_results[0].first == first){
                    std::size_t b = longest_last - longest_first, a = match_results[0].str().size();
                    if(a > b){
                        longest_last = first + a;
                        current_term = e.first;
                        match = true;
                    }
                }
            }
            if(match){
                first = longest_last;
                std::size_t char_count_before = char_count, line_count_before = line_count;
                for(auto &i : vstring_range(longest_first, longest_last)){
                    if(i == '\n'){
                        ++line_count;
                        char_count = 0;
                    }else if(i == '\t'){
                        char_count = (char_count / tab_width + 1) * tab_width;
                    }else{
                        ++char_count;
                    }
                }
                if(current_term != whitespace_functor()()){
                    token_type t;
                    t.value = vstring_range(longest_first, longest_last);
                    t.term = current_term;
                    t.char_num = char_count_before;
                    t.word_num = char_count - char_count_before;
                    t.line_num = line_count_before;
                    token_seq_subst.push_back(t);
                }
            }else{
                throw scanning_exception("lexical error.", char_count, 0, line_count);
            }
        }
        token_type t;
        t.term = eos_functor()();
        token_seq_subst.push_back(t);
    }

    ast::ast(token_type const &token, std::vector<ast*> const &nodes) : token(token), nodes(nodes){}

    ast::~ast(){
        for(auto ptr : nodes){
            delete ptr;
        }
    }

    scanning_data_type::~scanning_data_type(){
        for(auto ptr : ast_stack){
            delete ptr;
        }
    }

    ast const *scanning_data_type::get_arg_opt(ast const *ptr){
        if(ptr->token.value.empty()){
            return ast::dummy_storage_ptr();
        }else{
            return ptr->nodes[1];
        }
    }

    ast const *scanning_data_type::get_semantic_action(ast const *ptr){
        if(ptr->nodes[1]->token.value.empty()){
            return ast::dummy_storage_ptr();
        }else{
            return ptr->nodes[1]->nodes[0];
        }
    }

    ast const *scanning_data_type::get_tag_opt(ast const *ptr){
        if(ptr->token.value.empty()){
            return ast::dummy_storage_ptr();
        }else{
            return ptr->nodes[1];
        }
    }

    void scanning_data_type::get_rhs_seq(ast const *ptr, rhs_seq &seq){
        ast const *identifier, *arg;
        if(ptr->nodes.size() == 3){
            get_rhs_seq(ptr->nodes[0], seq);
            identifier = ptr->nodes[1];
            arg = get_arg_opt(ptr->nodes[2]);
        }else{
            identifier = ptr->nodes[0];
            arg = get_arg_opt(ptr->nodes[1]);
        }
        rhs_seq_element_type rhs_seq_element;
        rhs_seq_element.identifier = identifier->token;
        rhs_seq_element.arg = arg->token;
        rhs_seq_element.display_pos = current_scanning_rhs_token_count++;
        seq.push_back(rhs_seq_element);
    }

    scanning_data_type::rhs_seq scanning_data_type::get_rhs_seq_opt(ast const *ptr){
        rhs_seq s;
        if(!ptr->token.value.empty()){
            get_rhs_seq(ptr->nodes[0], s);
        }
        return s;
    }

    void scanning_data_type::get_rhs(ast const *ptr, rhs_seq_set &set){
        ast const *semantic_action, *tag;
        rhs_seq seq;
        if(ptr->nodes.size() == 5){
            get_rhs(ptr->nodes[0], set);
            semantic_action = get_semantic_action(ptr->nodes[2]);
            tag = get_tag_opt(ptr->nodes[3]);
            seq = get_rhs_seq_opt(ptr->nodes[4]);
        }else{
            semantic_action = get_semantic_action(ptr->nodes[0]);
            tag = get_tag_opt(ptr->nodes[1]);
            seq = get_rhs_seq_opt(ptr->nodes[2]);
        }
        current_scanning_rhs_token_count = 0;
        seq.semantic_action = semantic_action->token;
        seq.tag = tag->token;
        auto p = set.insert(seq);
        if(!p.second){
            expr_statements_error.push_back(scanning_exception("duplicated rhs.", ptr->token.char_num, ptr->token.word_num, ptr->token.line_num));
        }
    }

    void scanning_data_type::get_expr(ast const *ptr){
        rhs_seq_set s;
        get_rhs(ptr->nodes[2], s);
        auto p = rules.insert(std::make_pair(ptr->nodes[0]->nodes[0]->token, s));
        if(!p.second){
            expr_statements_error.push_back(scanning_exception("duplicated rule", ptr->nodes[0]->token.char_num, ptr->nodes[0]->token.word_num, ptr->nodes[0]->token.line_num));
        }
    }

    void scanning_data_type::get_expr_statements(ast const *ptr){
        if(ptr->nodes[0]->token.term == nt("ExprStatements")){
            get_expr_statements(ptr->nodes[0]);
            if(ptr->nodes.size() == 3){
                get_expr(ptr->nodes[1]);
            }
        }else if(ptr->nodes[0]->token.term == nt("Expr")){
            get_expr(ptr->nodes[0]);
        }
    }

    void scanning_data_type::get_top_level_seq_statements(ast const *ptr){
        if(ptr->nodes[0]->token.term == semicolon){
            return;
        }
        if(ptr->nodes[0]->token.term == nt("TopLevelSeqStatementsElement")){
            get_top_level_seq_statements_element(ptr->nodes[0]);
        }else if(ptr->nodes[0]->token.term == nt("TopLevelSeqStatements")){
            get_top_level_seq_statements(ptr->nodes[0]);
            if(ptr->nodes[1]->token.term != semicolon){
                get_top_level_seq_statements_element(ptr->nodes[1]);
            }
        }
    }

    void scanning_data_type::get_top_level_seq_statements_element(ast const *ptr){
        if(ptr->nodes.size() == 2){
            get_identifier_seq(ptr->nodes[0], linkdir::nonassoc);
        }else{
            get_block_with_link_dir(ptr->nodes[0]);
        }
    }

    void scanning_data_type::get_block_with_link_dir(ast const *ptr){
        get_seq_statements(ptr->nodes[2], get_link_dir(ptr->nodes[0]));
    }

    scanning_data_type::linkdir scanning_data_type::get_link_dir(ast const *ptr){
        if(ptr->nodes[1]->token.value == "nonassoc"){
            return linkdir::nonassoc;
        }else if(ptr->nodes[1]->token.value == "left"){
            return linkdir::left;
        }else if(ptr->nodes[1]->token.value == "right"){
            return linkdir::right;
        }
        throw(scanning_exception("link direction.", ptr->nodes[1]->token.char_num, ptr->nodes[1]->token.word_num, ptr->nodes[1]->token.line_num));
    }

    void scanning_data_type::get_seq_statements(ast const *ptr, linkdir dir){
        if(ptr->nodes.size() < 2){
            return;
        }
        if(ptr->nodes[0]->token.term == nt("SeqStatementsElement")){
            get_seq_statements_element(ptr->nodes[0], dir);
        }else if(ptr->nodes[0]->token.term == nt("SeqStatements")){
            get_seq_statements(ptr->nodes[0], dir);
            if(ptr->nodes.size() == 3){
                get_seq_statements_element(ptr->nodes[1], dir);
            }
        }
    }

    void scanning_data_type::get_seq_statements_element(ast const *ptr, linkdir dir){
        get_identifier_seq(ptr->nodes[0], dir);
        --current_token_priority;
    }

    void scanning_data_type::get_identifier_seq(ast const *ptr, linkdir dir){
        ast const *identifier;
        if(ptr->nodes.size() == 3){
            get_identifier_seq(ptr->nodes[0], dir);
            identifier = ptr->nodes[2];
        }else{
            identifier = ptr->nodes[0];
        }
        symbol_data_type symbol_data;
        symbol_data.priority = current_token_priority;
        symbol_data.dir = dir;
        std::pair<symbol_data_map_type::const_iterator, bool> p = symbol_data_map.insert(std::make_pair(identifier->token, symbol_data));
        if(!p.second){
            identifier_seq_error.push_back(scanning_exception("token.", identifier->token.char_num, identifier->token.word_num, identifier->token.line_num));
        }else{
            ordered_token_iters.push_back(p.first);
        }
    }

    void scanning_data_type::get_regexp_statements(ast const *ptr){
        if(ptr->nodes.size() == 1){
            if(ptr->nodes[0]->token.term != semicolon){
                get_regexp_seq(ptr->nodes[0]);
            }
        }else{
            get_regexp_statements(ptr->nodes[0]);
            if(ptr->nodes[1]->token.term != semicolon){
                get_regexp_seq(ptr->nodes[1]);
            }
        }
    }

    void scanning_data_type::get_regexp_seq(ast const *ptr){
        ast const *action = nullptr, *identifier, *symbol_str;
        if(ptr->nodes[0]->nodes.size() > 0){
            action = ptr->nodes[0]->nodes[1];
        }
        identifier = ptr->nodes[1];
        symbol_str = ptr->nodes[3];
        regexp_symbol_data_type data;
        data.action = action;
        data.priority = current_regexp_token_priority++;
        data.regexp = symbol_str->token.value;
        regexp_symbol_data_map.insert(std::make_pair(identifier->token, data));
    }

    scanning_data_type scanning_data;

    void scanning_data_type::collect_info(){
        get_regexp_statements(regexp_body);
        std::map<std::size_t, std::pair<const token_type, regexp_symbol_data_type> const*> sorted_regexp_map;
        for(auto &iter : regexp_symbol_data_map){
            sorted_regexp_map.insert(std::make_pair(iter.second.priority, &iter));
        }
        for(auto &iter : sorted_regexp_map){
            std::string str = iter.second->second.regexp.to_str();
            automaton_lexer.add_rule(
                str.substr(1, str.size() - 2),
                iter.second->first.value.to_str(),
                iter.second->second.action
                    ? iter.second->second.action->token.value.to_str()
                    : ""
            );
        }

        get_top_level_seq_statements(token_body);
        if(identifier_seq_error.size() > 0){
            throw(identifier_seq_error);
        }

        lalr_generator_type::item s;
        static vstring s_prime = { 'S', '\'' };
        s.lhs = lalr_generator.symbol_manager.set_nonterminal(vstring_range(s_prime.begin(), s_prime.end()));
        s.rhs.push_back(-2);
        s.pos = 0;
        lalr_generator.grammar.insert(std::make_pair(s.lhs, lalr_generator_type::rule_rhs({ s.rhs })));

        std::map<std::string, automaton::lexer::token_info*> translator_token_to_lexer_map;
        for(auto &info : automaton_lexer.token_info_vector){
            translator_token_to_lexer_map[info.name] = &info;
        }
        {
            std::size_t iterate_count = 0;
            for(auto iter : ordered_token_iters){
                lalr_generator_type::symbol_data_type symbol_data;
                symbol_data.priority = iter->second.priority;
                using linkdir_type = lalr_generator_type::linkdir;
                symbol_data.dir = iter->second.dir == linkdir::nonassoc ? linkdir_type::nonassoc : iter->second.dir == linkdir::left ? linkdir_type::left : linkdir_type::right;
                lalr_generator.symbol_data_map.insert(std::make_pair(lalr_generator.symbol_manager.set_terminal(iter->first.value), symbol_data));
                if(translator_token_to_lexer_map.find(iter->first.value.to_str()) == translator_token_to_lexer_map.end()){
                    lalr_generator.symbol_manager.set_terminal(iter->first.value);
                }else{
                    translator_token_to_lexer_map[iter->first.value.to_str()]->identifier = iterate_count + 1;
                }
                ++iterate_count;
            }
        }

        get_expr_statements(expr_statements);
        if(identifier_seq_error.size() > 0){
            throw expr_statements_error;
        }

        for(auto &iter : rules){
            lalr_generator_type::rule_rhs &rhs = lalr_generator.grammar[lalr_generator.symbol_manager.set_nonterminal(iter.first.value)];
            for(auto &seq : iter.second){
                seq.make_arg_to_element_map();
            }
        }

        automaton_lexer.build();

        {
            scanning_exception_seq exception_seq;

            std::set<token_type> token_map;
            for(auto &iter : ordered_token_iters){
                token_map.insert(iter->first);
            }

            std::set<token_type> lhs_map;
            for(auto &iter : rules){
                lhs_map.insert(iter.first);
            }

            for(auto &iter : rules){
                for(auto &seq : iter.second){
                    std::set<std::size_t> sequential_check;
                    for(auto &term_iter : seq){
                        if(!seq.tag.value.to_str().empty()){
                            if(token_map.find(seq.tag) == token_map.end()){
                                exception_seq.push_back(scanning_exception("'" + seq.tag.value.to_str() + "', tag is not found.", seq.tag.char_num, seq.tag.word_num, seq.tag.line_num));
                            }
                        }

                        auto token_map_iter = token_map.find(term_iter.identifier);
                        auto lhs_map_iter = lhs_map.find(term_iter.identifier);
                        if(token_map_iter == token_map.end() && lhs_map_iter == lhs_map.end()){
                            exception_seq.push_back(scanning_exception("undefined token, '" + term_iter.identifier.value.to_str() + "'.", term_iter.identifier.char_num, term_iter.identifier.word_num, term_iter.identifier.line_num));
                            continue;
                        }
                        if(!term_iter.arg.value.to_str().empty()){
                            sequential_check.insert(std::atoi(term_iter.arg.value.to_str().c_str()));
                        }
                    }
                    if(sequential_check.size() > 0){
                        std::size_t i = 0;
                        bool error = false;
                        for(auto sequential_check_iter = sequential_check.begin(); sequential_check_iter != sequential_check.end(); ++i, ++sequential_check_iter){
                            error = i != *sequential_check_iter;
                            if(error){
                                break;
                            }
                        }
                        if(error){
                            std::string str;
                            for(auto &term_iter : seq){
                                str += term_iter.identifier.value.to_str() + " ";
                            }
                            str.resize(str.size() - 1);
                            exception_seq.push_back(scanning_exception("arguments is not sequential, '" + str + "'", iter.first.char_num, iter.first.word_num, iter.first.line_num));
                            continue;
                        }
                    }
                }
            }

            if(!exception_seq.empty()){
                throw exception_seq;
            }
        }

        {
            scanning_exception_seq exception_seq;
            for(auto &iter : regexp_symbol_data_map){
                if(symbol_data_map.find(iter.first) == symbol_data_map.end()){
                    exception_seq.push_back(scanning_exception("unused regexp '" + iter.first.value.to_str() + "'.", iter.first.char_num, iter.first.word_num, iter.first.line_num));
                }
            }

            for(auto &iter : rules){
                for(auto &term_seq : iter.second){
                    for(auto &symbol : term_seq){
                        if(symbol.arg.value.empty()){
                            continue;
                        }
                        if(lalr_generator_type::is_terminal_symbol_functor()(lalr_generator.symbol_manager.get(symbol.identifier.value), rules)){
                            if(translator_token_to_lexer_map.find(symbol.identifier.value.to_str())->second->action.empty()){
                                exception_seq.push_back(scanning_exception("'" + symbol.identifier.value.to_str() + "' has not action.", iter.first.char_num, iter.first.word_num, iter.first.line_num));
                            }
                        }
                    }
                }
            }

            if(!exception_seq.empty()){
                throw exception_seq;
            }
        }

        for(auto &iter : rules){
            lalr_generator_type::rule_rhs &rhs = lalr_generator.grammar[lalr_generator.symbol_manager.set_nonterminal(iter.first.value)];
            for(auto &seq : iter.second){
                lalr_generator_type::term_sequence term_sequence;
                for(auto &kter : seq){
                    term_sequence.push_back(lalr_generator.symbol_manager.get(kter.identifier.value));
                }
                term_sequence.semantic_data.arg_to_element = &seq.arg_to_element;
                term_sequence.semantic_data.action = seq.semantic_action.value;
                if(!seq.tag.value.empty()){
                    term_sequence.tag = lalr_generator.symbol_manager.get(seq.tag.value);
                }
                rhs.insert(term_sequence);
            }
        }

        lalr_generator_type::states::iterator first_state;
        lalr_generator_type::term_set terminal_symbol_set = lalr_generator_type::make_terminal_symbol_set(lalr_generator.grammar);
        lalr_generator.make_follow_set(lalr_generator.grammar, s.lhs);
        lalr_generator.lr0_kernel_items(lalr_generator.grammar, states_prime, states, first_state, terminal_symbol_set, s);
        lalr_generator.make_goto_map(lalr_generator.grammar, terminal_symbol_set, states_prime, states, s);
        lalr_generator.completion_lookahead(lalr_generator.grammar, states, first_state, s);

        lalr_generator_type::items::iterator first_item;
        states_prime.clear();
        states = lalr_generator.c_closure(lalr_generator.grammar, states, first_state, first_item);
        lalr_generator_make_result = lalr_generator.make2(lalr_generator.grammar, states, first_state, *first_item, lalr_generator.symbol_data_map);
        if(lalr_generator_make_result.conflict_set.size() > 0){
            lalr_generator_type::exception_seq exception_seq;
            for(auto &i : lalr_generator_make_result.conflict_set){
                std::string act_str[2];
                lalr_generator_type::lr_parsing_table_item::enum_action act[2] = {
                    i.lhs.action,
                    i.rhs.action
                };
                for(int n = 0; n < 2; ++n){
                    switch(act[n]){
                    case lalr_generator_type::lr_parsing_table_item::enum_action::accept:
                        act_str[n] = "acc";
                        break;

                    case lalr_generator_type::lr_parsing_table_item::enum_action::shift:
                        act_str[n] = "sft";
                        break;

                    case lalr_generator_type::lr_parsing_table_item::enum_action::reduce:
                        act_str[n] = "rdc";
                        break;
                    }
                }
                exception_seq.push_back(
                    std::runtime_error(
                        act_str[0] + "/" + act_str[1] + " conflict, " +
                        lalr_generator.symbol_manager.to_str(i.lhs.item_ptr->lhs).to_str() + " vs " + lalr_generator.symbol_manager.to_str(i.rhs.item_ptr->lhs).to_str() + "."
                    )
                );
            }
            throw exception_seq;
        }
    }

    std::vector<vstring_range> make_signature(
        vstring_range const &semantic_action,
        vstring_range const &return_type,
        std::map<std::size_t, const rhs_seq_element_type*> const &arg_to_symbol_map
    ){
        std::vector<vstring_range> signature = { semantic_action, return_type };
        for(std::size_t i = 0; i < arg_to_symbol_map.size(); ++i){
            signature.push_back(arg_to_symbol_map.find(i)->second->identifier.value);
        }
        return signature;
    }

    void scanning_data_type::generate_cpp_semantic_data(std::ostream &os){
        os << R"text(#ifndef LXQ_HPP_
#define LXQ_HPP_

namespace lxq{
    template<class T = void>
    class semantic_data_proto{
    public:
        virtual ~semantic_data_proto() = default;
    };

    using semantic_data = semantic_data_proto<>;
}

#endif
)text";
    }

    void scanning_data_type::generate_cpp(std::ostream &os){
        std::string include_guard = grammar_namespace->token.value.to_str();
        for(auto &c : include_guard){
            if(std::islower(c)){
                c = std::toupper(c);
            }
        }
        os << "#ifndef " << include_guard << "_HPP_\n";
        os << "#define " << include_guard << "_HPP_\n";

        os << R"text(
#include <functional>
#include <exception>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cassert>
)text";
        os << "\n";
        os << "#include \"lxq.hpp\"\n";
        os << "#include \"" << regexp_namespace->token.value.to_str() << ".hpp\"\n\n";

        os << "namespace " << grammar_namespace->token.value.to_str() << "{";
        os << R"text(
    using semantic_data = lxq::semantic_data;

    // parser
    template<class Lexer, class SemanticDataProc>
    class parser{
    private:
        using term = int;
        using token_type = typename Lexer::token_type;
        using arg_type = std::vector<std::unique_ptr<semantic_data>>;
        using call_function = std::function<std::unique_ptr<semantic_data>(parser&, arg_type const&)>;

        struct term_sequence_data{
            std::size_t norm;
            call_function call;
        };

        struct parsing_table_item{
            enum class enum_action{
                shift,
                reduce,
                accept
            };

            enum_action action;
            std::size_t num;
        };

        struct parsing_data{
            std::size_t first;
            std::map<std::size_t, std::pair<term, term_sequence_data>> n2r;
            std::map<std::size_t, std::map<term, parsing_table_item>> parsing_table;
            std::map<std::size_t, std::map<term, std::size_t>> goto_table;
        };

        static parsing_data const &parsing_data_storage(){
            auto init = [](){
                return parsing_data{
                    )text";
        os << lalr_generator_make_result.first << ", // first";

        os << R"text(
                    // n2r
                    decltype(parsing_data::n2r){)text";
        {
            std::size_t iterate_count = 0;
            for(auto &iter : lalr_generator_make_result.n2r){
                if(iter.second.first == -1){
                    continue;
                }
                os << R"text(
                        std::make_pair(
                            )text";
                os << iter.first << ",";
                os << R"text(
                            std::make_pair()text";
                os << iter.second.first << ", term_sequence_data{ " << iter.second.second->size() << ", [](parser &p, arg_type const &arg){ return call_" << iter.second.second->semantic_data.action.to_str() << "(p, ";
                {
                    auto *p = iter.second.second->semantic_data.arg_to_element;
                    if(p){
                        for(std::size_t i = 0; i < p->size(); ++i){
                            os << "arg[" << p->at(i)->display_pos << "]";
                            if(i + 1 != p->size()){
                                os << ", ";
                            }
                        }
                    }
                }
                os << "); } })";
                os << R"text(
                        ))text";
                if(iterate_count + 1 < lalr_generator_make_result.n2r.size()){
                    os << ",";
                }
                ++iterate_count;
            }
        }
        os << R"text(
                    },

                    // parsing_table
                    decltype(parsing_data::parsing_table){)text";
        {
            std::size_t iterate_count = 0;
            for(auto &iter : lalr_generator_make_result.parsing_table){
                os << R"text(
                        std::make_pair(
                            )text";
                os << iter.first << ",";
                os << R"text(
                            std::map<term, parsing_table_item>{)text";
                {
                    std::size_t iterate_count = 0;
                    for(auto &jter : iter.second){
                        os << R"text(
                                std::make_pair()text";
                        os << jter.first << ", parsing_table_item{ ";
                        switch(jter.second.action){
                        case lalr_generator_type::lr_parsing_table_item::enum_action::shift:
                            os << "parsing_table_item::enum_action::shift";
                            break;

                        case lalr_generator_type::lr_parsing_table_item::enum_action::reduce:
                            os << "parsing_table_item::enum_action::reduce";
                            break;

                        case lalr_generator_type::lr_parsing_table_item::enum_action::accept:
                            os << "parsing_table_item::enum_action::accept";
                            break;
                        }
                        os << ", " << jter.second.num << " })";
                        if(iterate_count + 1 != iter.second.size()){
                            os << ",";
                        }
                        ++iterate_count;
                    }
                }
                os << R"text(
                            }
                        ))text";
                if(iterate_count + 1 != lalr_generator_make_result.parsing_table.size()){
                    os << ",";
                }
                ++iterate_count;
            }
        }
        os << R"text(
                    },

                    // goto_table
                    decltype(parsing_data::goto_table){)text";
        {
            std::size_t iterate_count = 0;
            for(auto &iter : lalr_generator_make_result.goto_table){
                os << R"text(
                        std::make_pair(
                            )text";
                os << iter.first << ",";
                os << R"text(
                            std::map<term, std::size_t>{)text";
                {
                    std::size_t iterate_count = 0;
                    for(auto &jter : iter.second){
                        os << R"text(
                                )text";
                        os << "std::make_pair(" << jter.first << ", " << jter.second << ")";
                        if(iterate_count + 1 != iter.second.size()){
                            os << ",\n";
                        }
                        ++iterate_count;
                    }
                }
                if(iter.second.empty()){
                    os << R"text(}
                        ))text";
                }else{
                    os << R"text(
                            }
                        ))text";
                }
                if(iterate_count + 1 != lalr_generator_make_result.goto_table.size()){
                    os << ",";
                }
                ++iterate_count;
            }
        }
        os << R"text(
                    }
                };
            };
            static parsing_data data = init();
            return data;
        }

)text";
        std::set<vstring_range> generated_callfn;
        for(auto &iter : lalr_generator_make_result.n2r){
            if(iter.second.first == -1 || generated_callfn.find(iter.second.second->semantic_data.action) != generated_callfn.end()){
                continue;
            }
            generated_callfn.insert(iter.second.second->semantic_data.action);
            auto *p = iter.second.second->semantic_data.arg_to_element;
            os << "        static std::unique_ptr<semantic_data> call_" << iter.second.second->semantic_data.action.to_str() << "(parser &p,";
            if(p){
                for(std::size_t i = 0; i < p->size(); ++i){
                    os << "std::unique_ptr<semantic_data> const &v_" << i;
                    if(i + 1 != p->size()){
                        os << ", ";
                    }
                }
            }
            os << "){";
            os << R"text(
            return std::move(std::unique_ptr<semantic_data>(p.semantic_data_proc.)text";
            os << iter.second.second->semantic_data.action.to_str() << "(";
            if(p){
                for(std::size_t i = 0; i < p->size(); ++i){
                    os << "v_" << i;
                    if(i + 1 != p->size()){
                        os << ".get(), ";
                    }else{
                        os << ".get()";
                    }
                }
            }
            os << ")));";
            os << R"text(
        }

)text";
        }
        os << R"text(    public:
        SemanticDataProc &semantic_data_proc;

        parser() = delete;
        parser(SemanticDataProc &semantic_data_proc) : semantic_data_proc(semantic_data_proc){}

        template<class InputIter>
        InputIter parse(std::unique_ptr<semantic_data> &value, InputIter first, InputIter last){
            parsing_data const &table = parsing_data_storage();
            std::vector<std::size_t> state_stack;
            std::vector<std::unique_ptr<semantic_data>> value_stack;
            state_stack.push_back(table.first);
            while(true){
                token_type &token = *first;
                term const &t = static_cast<term>(token.identifier);
                std::size_t s = state_stack.back();
                auto const &table_second(table.parsing_table.find(s)->second);
                auto iter = table_second.find(t);
                if(iter == table_second.end()){
                    break;
                }
                parsing_table_item const &i = table.parsing_table.find(s)->second.find(t)->second;
                if(i.action == parsing_table_item::enum_action::shift){
                    state_stack.push_back(i.num);
                    value_stack.push_back(std::unique_ptr<semantic_data>(nullptr));
                    value_stack.back().swap(token.value);
                    ++first;
                }else if(i.action == parsing_table_item::enum_action::reduce){
                    auto &p = *table.n2r.find(i.num);
                    std::size_t norm = p.second.second.norm;
                    state_stack.resize(state_stack.size() - norm);
                    if(state_stack.empty()){
                        break;
                    }
                    std::vector<std::unique_ptr<semantic_data>> arg;
                    arg.reserve(norm);
                    for(std::size_t i = 0; i < norm; ++i){
                        arg.push_back(std::unique_ptr<semantic_data>(nullptr));
                        arg.back().swap(std::move(value_stack[value_stack.size() - norm + i]));
                    }
                    value_stack.resize(value_stack.size() - norm);
                    value_stack.push_back(std::move(p.second.second.call(*this, arg)));
                    state_stack.push_back(table.goto_table.find(state_stack.back())->second.find(p.second.first)->second);
                }else if(i.action == parsing_table_item::enum_action::accept){
                    if(value_stack.size() != 1){
                        break;
                    }
                    value = std::move(value_stack.front());
                    ++first;
                    break;
                }
            }
            return first;
        }
    };
}

#endif
)text";
    }

    bool rhs_seq_element_type::operator <(rhs_seq_element_type const &other) const{
        if(identifier.value < other.identifier.value){
            return true;
        }else if(other.identifier.value < identifier.value){
            return false;
        }else if(arg.value < other.arg.value){
            return true;
        }else{
            return false;
        }
    }

    void scanning_data_type::rhs_seq::make_arg_to_element_map() const{
        max_arg = 0;
        for(auto &i : *this){
            if(i.arg.value.empty()){
                continue;
            }
            std::size_t n = std::atoi(i.arg.value.to_str().c_str());
            arg_to_element.insert(std::make_pair(n, &i));
            if(n > max_arg){
                max_arg = n;
            }
        }
    }

    void scan(const std::string ifile_path, std::string out_path){
        try{
            scanner::grammar grammar;
            init_grammar(grammar);
            scanner sc;
            scanner::symbol_data_map symbol_data_map;
            scanner::term_set terminal_symbol_set = scanner::make_terminal_symbol_set(grammar);
            scanner::item s;
            s.lhs = symbol_manager.set_nonterminal("S'");
            s.rhs.push_back(symbol_manager.set_nonterminal("Start"));
            s.pos = 0;
            s.lookahead.insert(eos_functor()());

            sc.make_follow_set(grammar, s.lhs);
            scanner::states states_prime, states;
            scanner::states::iterator first_state = states.end();
            sc.lr0_kernel_items(grammar, states_prime, states, first_state, terminal_symbol_set, s);
            sc.make_goto_map(grammar, terminal_symbol_set, states_prime, states, s);
            sc.completion_lookahead(grammar, states, first_state, s);

            scanner::items::iterator first_item;
            states_prime.clear();
            states = sc.c_closure(grammar, states, first_state, first_item);
            scanner::make_result make_result = sc.make2(grammar, states, states.end(), s, symbol_data_map);
            if(!make_result.conflict_set.empty()){
                throw std::runtime_error("bootstrap parser parsing error.");
            }

            std::ifstream ifile(ifile_path, std::ios::binary);
            if(!ifile){
                throw std::runtime_error("cannot open input file.");
            }

            std::ifstream::streampos tell_beg, tell_eof;
            ifile.seekg(0, std::ifstream::end);
            tell_eof = ifile.tellg();
            ifile.clear();
            ifile.seekg(0, std::ifstream::beg);
            tell_beg = ifile.tellg();
            std::size_t ifile_size = static_cast<std::size_t>(tell_eof - tell_beg);
            vstring string(ifile_size);
            ifile.read(string.data(), ifile_size);

            lexer lex;
            init_lexer(lex);

            lex.tokenize(string.begin(), string.end());
            sc.parse(make_result, lex.token_seq.begin(), lex.token_seq.end() - 1);
            scanning_data.collect_info();

            if(out_path.empty()){
                out_path += "./";
            }else if(out_path.back() != '/'){
                out_path += "/";
            }

            std::string lxq_file_name = "lxq.hpp";
            std::string lxq_file_path = out_path + lxq_file_name;
            std::ofstream lxq_hpp(lxq_file_path);
            if(lxq_hpp.fail()){
                throw std::runtime_error("can not create lxq.hpp file.");
            }

            std::string lexer_file_name = scanning_data.regexp_namespace->token.value.to_str() + ".hpp";
            std::string lexer_file_path = out_path + lexer_file_name;
            std::ofstream lexer_hpp(lexer_file_path);
            if(lexer_hpp.fail()){
                throw std::runtime_error("can not create lexer.hpp file.");
            }
            
            std::string parser_file_name = scanning_data.grammar_namespace->token.value.to_str() + ".hpp";
            std::string parser_file_path = out_path + parser_file_name;
            std::ofstream grammar_hpp(parser_file_path);
            if(grammar_hpp.fail()){
                throw std::runtime_error("can not create parser.hpp file.");
            }

            scanning_data.generate_cpp_semantic_data(lxq_hpp);
            scanning_data.automaton_lexer.generate_cpp(lexer_hpp, scanning_data.regexp_namespace->token.value.to_str());
            scanning_data.generate_cpp(grammar_hpp);
        }catch(std::runtime_error e){
            std::cout << e.what() << std::endl;
            return;
        }catch(scanning_exception e){
            std::cout << e.message << " : line " << e.line_num << ", char " << e.char_num << std::endl;
            return;
        }catch(scanning_exception_seq seq){
            for(scanning_exception &e : seq){
                std::cout << e.message << " : line " << e.line_num << ", char " << e.char_num << std::endl;
            }
            return;
        }
        return;
    }
}
