#include "scanner.hpp"

namespace scanner{
    term_type nt(std::string const &str){
        return symbol_manager.set_nonterminal(str);
    }

    void init_lexer(lexer &lex){
        lex.new_regex("( |\t|\r|\n|\r\n|(//[^\r\n]*(\r|\n|\r\n)))+", whitespace_functor()());
        lex.new_regex("[0-9]+", value);
        lex.new_regex(",", comma);
        lex.new_regex("\\.", dot);
        lex.new_regex("\\?", question);
        lex.new_regex("!", exclamation);
        lex.new_regex("\\+", plus);
        lex.new_regex("-", hyphen);
        lex.new_regex("\\*", asterisk);
        lex.new_regex("\\/", slash);
        lex.new_regex(":", colon);
        lex.new_regex(";", semicolon);
        lex.new_regex("\\[", l_square_bracket);
        lex.new_regex("\\]", r_square_bracket);
        lex.new_regex("\\{", l_curly_bracket);
        lex.new_regex("\\}", r_curly_bracket);
        lex.new_regex("<", l_bracket);
        lex.new_regex(">", r_bracket);
        lex.new_regex("\\(", l_round_paren);
        lex.new_regex("\\)", r_round_paren);
        lex.new_regex("\\|", vertical_bar);
        lex.new_regex("=", equal);
        lex.new_regex("\"([^\"]|\\\\\")+\"", string);
        lex.new_regex("[a-zA-Z_][a-zA-Z0-9_]*", identifier);
    }

    void init_grammar(scanner::grammar &grammar){
        using seq = scanner::term_sequence;

        auto decl_g = [&](std::string const &str) -> scanner::rule_rhs&{
            return grammar[symbol_manager.set_nonterminal(str)];
        };

        semantic_type eat = [](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type token;
            token.term = term;
            if(arg.size() > 0){
                bool exists = false;
                auto arg_head_iter = arg.begin();
                for(; arg_head_iter != arg.end(); ++arg_head_iter){
                    if(!arg_head_iter->value.empty()){
                        exists = true;
                        break;
                    }
                }
                auto arg_tail_iter = arg.rbegin();
                for(; arg_tail_iter != arg.rend(); ++arg_tail_iter){
                    if(!arg_tail_iter->value.empty()){
                        break;
                    }
                }
                if(exists){
                    token.value = vstring_range(arg_head_iter->value.begin(), arg_tail_iter->value.end());
                }
            }
            ast *a = new ast;
            a->nodes.resize(arg.size());
            std::size_t i = 0;
            for(auto iter = data.ast_stack.end() - arg.size(); iter != data.ast_stack.end(); ++iter, ++i){
                a->nodes[i] = *iter;
            }
            a->token = token;
            data.ast_stack.resize(data.ast_stack.size() - arg.size());
            data.ast_stack.push_back(a);
            return token; // auto
        };

        decl_g("RegExpSeq") = scanner::rule_rhs({
            seq({ nt("RegExpAction_opt"), identifier, equal, string }, eat)
        });

        decl_g("RegExpAction_opt") = scanner::rule_rhs({
            seq({}, eat),
            seq({ l_square_bracket, identifier, r_square_bracket }, eat)
        });

        decl_g("IdentifierSeq") = scanner::rule_rhs({
            seq({ identifier }, eat),
            seq({ nt("IdentifierSeq"), comma, identifier }, eat)
        });

        decl_g("LinkDir") = scanner::rule_rhs({
            seq({ l_bracket, identifier, r_bracket }, eat)
        });

        decl_g("BlockWithLinkDir") = scanner::rule_rhs({
            seq({ nt("LinkDir"), l_curly_bracket, nt("SeqStatements"), r_curly_bracket }, eat)
        });

        decl_g("SeqStatements") = scanner::rule_rhs({
            seq({ nt("SeqStatementsElement"), semicolon }, eat),
            seq({ semicolon }, eat),
            seq({ nt("SeqStatements"), nt("SeqStatementsElement"), semicolon }, eat),
            seq({ nt("SeqStatements"), semicolon }, eat)
        });

        decl_g("SeqStatementsElement") = scanner::rule_rhs({
            seq({ nt("IdentifierSeq") }, eat)
        });

        decl_g("TopLevelSeqStatements") = scanner::rule_rhs({
            seq({ nt("TopLevelSeqStatementsElement") }, eat),
            seq({ semicolon }, eat),
            seq({ nt("TopLevelSeqStatements"), nt("TopLevelSeqStatementsElement") }, eat),
            seq({ nt("TopLevelSeqStatements"), semicolon }, eat),
        });

        decl_g("TopLevelSeqStatementsElement") = scanner::rule_rhs({
            seq({ nt("IdentifierSeq"), semicolon }, eat),
            seq({ nt("BlockWithLinkDir") }, eat)
        });

        decl_g("RegExpStatements") = scanner::rule_rhs({
            seq({ nt("RegExpSeq") }, eat),
            seq({ semicolon }, eat),
            seq({ nt("RegExpStatements"), nt("RegExpSeq") }, eat),
            seq({ nt("RegExpStatements"), semicolon }, eat),
        });

        decl_g("Arg_opt") = scanner::rule_rhs({
            seq({}, eat),
            seq({ l_round_paren, value, r_round_paren }, eat)
        });

        decl_g("SemanticAction") = scanner::rule_rhs({
            seq({ l_square_bracket, nt("SemanticActionElement_opt"), r_square_bracket }, eat)
        });

        decl_g("SemanticActionElement_opt") = scanner::rule_rhs({
            seq({}, eat),
            seq({ identifier }, eat)
        });

        decl_g("Tag_opt") = scanner::rule_rhs({
            seq({}, eat),
            seq({ l_bracket, identifier, r_bracket }, eat)
        });

        decl_g("RHSSeq") = scanner::rule_rhs({
            seq({ identifier, nt("Arg_opt") }, eat),
            seq({ nt("RHSSeq"), identifier, nt("Arg_opt") }, eat),
        });

        decl_g("RHSSeq_opt") = scanner::rule_rhs({
            seq({}, eat),
            seq({ nt("RHSSeq") }, eat)
        });

        decl_g("RHS") = scanner::rule_rhs({
            seq({ nt("SemanticAction"), nt("Tag_opt"), nt("RHSSeq_opt") }, eat),
            seq({ nt("RHS"), vertical_bar, nt("SemanticAction"), nt("Tag_opt"), nt("RHSSeq_opt") }, eat)
        });

        decl_g("LHS") = scanner::rule_rhs({
            seq({ identifier }, eat)
        });

        decl_g("Expr") = scanner::rule_rhs({
            seq({ nt("LHS"), colon, nt("RHS") }, eat)
        });

        decl_g("ExprStatements") = scanner::rule_rhs({
            seq({ nt("Expr"), semicolon }, eat),
            seq({ semicolon }, eat),
            seq({ nt("ExprStatements"), nt("Expr"), semicolon }, eat),
            seq({ nt("ExprStatements"), semicolon }, eat)
        });

        semantic_type regexp_header = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            if(data.ast_stack.back()->nodes[1]->token.value.to_str() != "lexer"){
                throw std::runtime_error("incorrect lexer header.");
            }
            return t;
        };
        decl_g("RegExpHeader") = scanner::rule_rhs({
            seq({ l_bracket, identifier, r_bracket }, regexp_header)
        });

        semantic_type regexp_namespace = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.regexp_namespace = data.ast_stack.back();
            return t;
        };
        decl_g("RegExpNamespace") = scanner::rule_rhs({
            seq({ identifier }, regexp_namespace)
        });

        semantic_type regexp_body = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.regexp_body = data.ast_stack.back()->nodes[1];
            return t;
        };
        decl_g("RegExpBody") = scanner::rule_rhs({
            seq({ l_curly_bracket, nt("RegExpStatements"), r_curly_bracket }, regexp_body)
        });

        semantic_type token_header = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            if(data.ast_stack.back()->nodes[1]->token.value.to_str() != "token"){
                throw std::runtime_error("incorrect token header.");
            }
            return t;
        };
        decl_g("TokenHeader") = scanner::rule_rhs({
            seq({ l_bracket, identifier, r_bracket }, token_header)
        });

        semantic_type token_namespace = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.token_namespace = data.ast_stack.back();
            return t;
        };
        decl_g("TokenNamespace") = scanner::rule_rhs({
            seq({ identifier }, token_namespace)
        });

        semantic_type token_body = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.token_body = data.ast_stack.back()->nodes[1];
            return t;
        };
        decl_g("TokenBody") = scanner::rule_rhs({
            seq({ l_curly_bracket, nt("TopLevelSeqStatements"), r_curly_bracket }, token_body)
        });

        semantic_type grammar_header = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            if(data.ast_stack.back()->nodes[1]->token.value.to_str() != "parser"){
                throw std::runtime_error("incorrect parser header.");
            }
            return t;
        };
        decl_g("GrammarHeader") = scanner::rule_rhs({
            seq({ l_bracket, identifier, r_bracket }, grammar_header)
        });

        semantic_type grammar_body = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.expr_statements = data.ast_stack.back()->nodes[1];
            return t;
        };
        decl_g("GrammarBody") = scanner::rule_rhs({
            seq({ l_curly_bracket, nt("ExprStatements"), r_curly_bracket }, grammar_body)
        });

        semantic_type grammar_namespace = [eat](term_type term, arg_type const &arg, scanning_data_type &data){
            token_type t = eat(term, arg, data);
            data.grammar_namespace = data.ast_stack.back();
            return t;
        };
        decl_g("GrammarNamespace") = scanner::rule_rhs({
            seq({ identifier }, grammar_namespace)
        });

        decl_g("Start") = scanner::rule_rhs({
            seq({
                nt("RegExpHeader"),
                nt("RegExpNamespace"),
                nt("RegExpBody"),
                nt("TokenHeader"),
                nt("TokenNamespace"),
                nt("TokenBody"),
                nt("GrammarHeader"),
                nt("GrammarNamespace"),
                nt("GrammarBody")
            }, eat)
        });

        decl_g("S'") = scanner::rule_rhs({
            seq({ nt("Start") }, eat)
        });
    }
}
