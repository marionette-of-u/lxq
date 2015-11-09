#ifndef REGEXP_PARSER_HPP_
#define REGEXP_PARSER_HPP_

#include <cstdlib>
#include <cassert>
#include <vector>

namespace regexp_parser{

enum regexp_token{
    regexp_token_symbol_or,
    regexp_token_symbol_star,
    regexp_token_symbol_plus,
    regexp_token_symbol_question,
    regexp_token_symbol_left_pare,
    regexp_token_symbol_right_pare,
    regexp_token_symbol_left_brace,
    regexp_token_symbol_right_brace,
    regexp_token_symbol_dot,
    regexp_token_symbol_eos,
    regexp_token_symbol_backslash,
    regexp_token_symbol_set_left_bracket,
    regexp_token_symbol_hat,
    regexp_token_symbol_set_right_bracket,
    regexp_token_symbol_minus,
    regexp_token_symbol_comma,
    regexp_token_symbol_colon,
    regexp_token_symbol_quote,
    regexp_token_symbol_any_non_metacharacter,
    regexp_token_symbol_number,
    regexp_token_0
};

template<class T, int StackSize>
class stack{
public:
    stack(){ gap_ = 0; }
    ~stack(){}
    
    void reset_tmp()
    {
        gap_ = stack_.size();
        tmp_.clear();
    }

    void commit_tmp()
    {
        // may throw
        stack_.reserve(gap_ + tmp_.size());

        // expect not to throw
        stack_.erase(stack_.begin() + gap_, stack_.end());
        stack_.insert(stack_.end(), tmp_.begin(), tmp_.end());
    }

    bool push(const T& f)
    {
        if(StackSize != 0 && StackSize <= stack_.size() + tmp_.size()){
            return false;
        }
        tmp_.push_back(f);
        return true;
    }

    void pop(std::size_t n)
    {
        if(tmp_.size() < n){
            n -= tmp_.size();
            tmp_.clear();
            gap_ -= n;
        }else{
            tmp_.erase(tmp_.end() - n, tmp_.end());
        }
    }

    const T& top()
    {
        if(!tmp_.empty()){
            return tmp_.back();
        } else {
            return stack_[gap_ - 1];
        }
    }

    const T& get_arg(std::size_t base, std::size_t index)
    {
        std::size_t n = tmp_.size();
        if(base - index <= n){
            return tmp_[n - (base - index)];
        }else{
            return stack_[gap_ - (base - n) + index];
        }
    }

    void clear()
    {
        stack_.clear();
    }

private:
    std::vector<T> stack_;
    std::vector<T> tmp_;
    std::size_t gap_;

};

template<class Value, class SemanticAction, int StackSize = 0>
class parser {
public:
    typedef regexp_token token_type;
    typedef Value value_type;

public:
    parser(SemanticAction& sa) : sa_(sa){ reset(); }

    void reset()
    {
        error_ = false;
        accepted_ = false;
        clear_stack();
        reset_tmp_stack();
        if(push_stack(&parser::state_0, &parser::gotof_0, value_type())){
            commit_tmp_stack();
        }else{
            sa_.stack_overflow();
            error_ = true;
        }
    }

    bool post(token_type token, const value_type& value)
    {
        assert(!error_);
        reset_tmp_stack();
        while((this->*(stack_top()->state))(token, value)); // may throw
        if(!error_){
            commit_tmp_stack();
        }
        return accepted_ || error_;
    }

    bool accept(value_type& v)
    {
        assert(accepted_);
        if(error_){ return false; }
        v = accepted_value_;
        return true;
    }

    bool error(){ return error_; }

private:
    typedef parser<Value, SemanticAction, StackSize> self_type;
    typedef bool (self_type::*state_type)(token_type, const value_type&);
    typedef bool ( self_type::*gotof_type )(int, const value_type&);

    bool            accepted_;
    bool            error_;
    value_type      accepted_value_;
    SemanticAction& sa_;

    struct stack_frame{
        state_type state;
        gotof_type gotof;
        value_type value;

        stack_frame(state_type s, gotof_type g, const value_type& v)
            : state(s), gotof(g), value(v){}
    };

    stack<stack_frame, StackSize> stack_;
    bool push_stack(state_type s, gotof_type g, const value_type& v)
    {
        bool f = stack_.push(stack_frame(s, g, v));
        assert(!error_);
        if(!f){
            error_ = true;
            sa_.stack_overflow();
        }
        return f;
    }

    void pop_stack(std::size_t n)
    {
        stack_.pop(n);
    }

    const stack_frame* stack_top()
    {
        return &stack_.top();
    }

    const value_type& get_arg(std::size_t base, std::size_t index)
    {
        return stack_.get_arg(base, index).value;
    }

    void clear_stack()
    {
        stack_.clear();
    }

    void reset_tmp_stack()
    {
        stack_.reset_tmp();
    }

    void commit_tmp_stack()
    {
        stack_.commit_tmp();
    }

    bool call_0_make_set_content(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_set_content(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_set_item(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_set_item(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_range(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_range(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_identity(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.identity(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_set_item(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_set_item(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_0(int nonterminal_index, int base)
    {
        value_type v = value_type();
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_class_content(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* r = sa_.make_class_content(arg0, arg1, arg2);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_char(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_char(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_char(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_char(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_char_seq(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_char_seq(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_1_make_char_seq(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_char_seq(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_union(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* r = sa_.make_union(arg0, arg1, arg2);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_eos(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_eos(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_after_nline(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_after_nline(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_any(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_any(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_str(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* r = sa_.make_str(arg0, arg1, arg2);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_set_or_class(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* r = sa_.make_set_or_class(arg0, arg1, arg2);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_group(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* r = sa_.make_group(arg0, arg1, arg2);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_m(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_m(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_elementary_regexp(int nonterminal_index, int base, int arg_index0)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* r = sa_.make_elementary_regexp(arg0);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_n_to_m(int nonterminal_index, int base, int arg_index0, int arg_index1, int arg_index2, int arg_index3, int arg_index4)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* arg2; sa_.downcast(arg2, get_arg(base, arg_index2));
        regexp_parser::regexp_ast* arg3; sa_.downcast(arg3, get_arg(base, arg_index3));
        regexp_parser::regexp_ast* arg4; sa_.downcast(arg4, get_arg(base, arg_index4));
        regexp_parser::regexp_ast* r = sa_.make_n_to_m(arg0, arg1, arg2, arg3, arg4);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_one_or_zero(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_one_or_zero(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_kleene_plus(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_kleene_plus(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_kleene(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_kleene(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool call_0_make_concat(int nonterminal_index, int base, int arg_index0, int arg_index1)
    {
        regexp_parser::regexp_ast* arg0; sa_.downcast(arg0, get_arg(base, arg_index0));
        regexp_parser::regexp_ast* arg1; sa_.downcast(arg1, get_arg(base, arg_index1));
        regexp_parser::regexp_ast* r = sa_.make_concat(arg0, arg1);
        value_type v = value_type();
        sa_.upcast(v, r);
        pop_stack(base);
        return (this->*(stack_top()->gotof))(nonterminal_index, v);
    }

    bool gotof_0(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_108, &parser::gotof_108, v);
        case 2: return push_stack(&parser::state_112, &parser::gotof_112, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 5: return push_stack(&parser::state_68, &parser::gotof_68, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 6: return push_stack(&parser::state_110, &parser::gotof_110, v);
        case 8: return push_stack(&parser::state_111, &parser::gotof_111, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 13: return push_stack(&parser::state_109, &parser::gotof_109, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_0(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_1(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_3, &parser::gotof_3, v);
        case 7: return push_stack(&parser::state_7, &parser::gotof_7, v);
        case 17: return push_stack(&parser::state_6, &parser::gotof_6, v);
        case 18: return push_stack(&parser::state_8, &parser::gotof_8, v);
        case 33: return push_stack(&parser::state_2, &parser::gotof_2, v);
        default: assert(0); return false;
        }
    }

    bool state_1(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_2(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_3, &parser::gotof_3, v);
        case 7: return push_stack(&parser::state_7, &parser::gotof_7, v);
        case 17: return push_stack(&parser::state_6, &parser::gotof_6, v);
        case 18: return push_stack(&parser::state_9, &parser::gotof_9, v);
        default: assert(0); return false;
        }
    }

    bool state_2(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        case regexp_token_symbol_set_right_bracket:
            return call_0_make_set_content(9, 2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_3(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_3(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_minus:
            // shift
            push_stack(&parser::state_4, &parser::gotof_4, value);
            return false;
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_set_item(18, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_4(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_5, &parser::gotof_5, v);
        default: assert(0); return false;
        }
    }

    bool state_4(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_5(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_5(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_range(17, 3, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_6(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_6(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_set_item(18, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_7(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_7(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_set_item(18, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_8(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_8(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_identity(33, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_9(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_9(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_1_make_set_item(33, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_10(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 9: return push_stack(&parser::state_15, &parser::gotof_15, v);
        case 30: return push_stack(&parser::state_74, &parser::gotof_74, v);
        case 31: return push_stack(&parser::state_16, &parser::gotof_16, v);
        case 32: return push_stack(&parser::state_1, &parser::gotof_1, v);
        default: assert(0); return false;
        }
    }

    bool state_10(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_11, &parser::gotof_11, value);
            return false;
        case regexp_token_symbol_colon:
            // shift
            push_stack(&parser::state_12, &parser::gotof_12, value);
            return false;
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_0(32, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_11(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_11(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_identity(32, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_12(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_81, &parser::gotof_81, v);
        case 21: return push_stack(&parser::state_13, &parser::gotof_13, v);
        default: assert(0); return false;
        }
    }

    bool state_12(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_13(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_82, &parser::gotof_82, v);
        default: assert(0); return false;
        }
    }

    bool state_13(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_colon:
            // shift
            push_stack(&parser::state_14, &parser::gotof_14, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_14(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_14(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_set_right_bracket:
            return call_0_make_class_content(31, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_15(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_15(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_set_right_bracket:
            return call_0_identity(30, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_16(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_16(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_set_right_bracket:
            return call_0_identity(30, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_17(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 29: return push_stack(&parser::state_80, &parser::gotof_80, v);
        default: assert(0); return false;
        }
    }

    bool state_17(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
            // shift
            push_stack(&parser::state_18, &parser::gotof_18, value);
            return false;
        case regexp_token_symbol_star:
            // shift
            push_stack(&parser::state_19, &parser::gotof_19, value);
            return false;
        case regexp_token_symbol_plus:
            // shift
            push_stack(&parser::state_20, &parser::gotof_20, value);
            return false;
        case regexp_token_symbol_question:
            // shift
            push_stack(&parser::state_21, &parser::gotof_21, value);
            return false;
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_22, &parser::gotof_22, value);
            return false;
        case regexp_token_symbol_right_pare:
            // shift
            push_stack(&parser::state_23, &parser::gotof_23, value);
            return false;
        case regexp_token_symbol_left_brace:
            // shift
            push_stack(&parser::state_24, &parser::gotof_24, value);
            return false;
        case regexp_token_symbol_right_brace:
            // shift
            push_stack(&parser::state_25, &parser::gotof_25, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_26, &parser::gotof_26, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_27, &parser::gotof_27, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_28, &parser::gotof_28, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_29, &parser::gotof_29, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_30, &parser::gotof_30, value);
            return false;
        case regexp_token_symbol_set_right_bracket:
            // shift
            push_stack(&parser::state_31, &parser::gotof_31, value);
            return false;
        case regexp_token_symbol_minus:
            // shift
            push_stack(&parser::state_32, &parser::gotof_32, value);
            return false;
        case regexp_token_symbol_comma:
            // shift
            push_stack(&parser::state_33, &parser::gotof_33, value);
            return false;
        case regexp_token_symbol_colon:
            // shift
            push_stack(&parser::state_34, &parser::gotof_34, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_35, &parser::gotof_35, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_36, &parser::gotof_36, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_37, &parser::gotof_37, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_18(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_18(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_19(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_19(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_20(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_20(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_21(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_21(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_22(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_22(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_23(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_23(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_24(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_24(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_25(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_25(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_26(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_26(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_27(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_27(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_28(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_28(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_29(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_29(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_30(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_30(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_31(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_31(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_32(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_32(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_33(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_33(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_34(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_34(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_35(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_35(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_36(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_36(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_37(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_37(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(29, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_38(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 28: return push_stack(&parser::state_61, &parser::gotof_61, v);
        default: assert(0); return false;
        }
    }

    bool state_38(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_39, &parser::gotof_39, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_40, &parser::gotof_40, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_39(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_39(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_identity(28, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_40(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_40(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_identity(28, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_41(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 25: return push_stack(&parser::state_72, &parser::gotof_72, v);
        case 26: return push_stack(&parser::state_42, &parser::gotof_42, v);
        case 27: return push_stack(&parser::state_62, &parser::gotof_62, v);
        default: assert(0); return false;
        }
    }

    bool state_41(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
            // shift
            push_stack(&parser::state_43, &parser::gotof_43, value);
            return false;
        case regexp_token_symbol_star:
            // shift
            push_stack(&parser::state_44, &parser::gotof_44, value);
            return false;
        case regexp_token_symbol_plus:
            // shift
            push_stack(&parser::state_45, &parser::gotof_45, value);
            return false;
        case regexp_token_symbol_question:
            // shift
            push_stack(&parser::state_46, &parser::gotof_46, value);
            return false;
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_47, &parser::gotof_47, value);
            return false;
        case regexp_token_symbol_right_pare:
            // shift
            push_stack(&parser::state_48, &parser::gotof_48, value);
            return false;
        case regexp_token_symbol_left_brace:
            // shift
            push_stack(&parser::state_49, &parser::gotof_49, value);
            return false;
        case regexp_token_symbol_right_brace:
            // shift
            push_stack(&parser::state_50, &parser::gotof_50, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_51, &parser::gotof_51, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_52, &parser::gotof_52, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_38, &parser::gotof_38, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_53, &parser::gotof_53, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_54, &parser::gotof_54, value);
            return false;
        case regexp_token_symbol_set_right_bracket:
            // shift
            push_stack(&parser::state_55, &parser::gotof_55, value);
            return false;
        case regexp_token_symbol_minus:
            // shift
            push_stack(&parser::state_56, &parser::gotof_56, value);
            return false;
        case regexp_token_symbol_comma:
            // shift
            push_stack(&parser::state_57, &parser::gotof_57, value);
            return false;
        case regexp_token_symbol_colon:
            // shift
            push_stack(&parser::state_58, &parser::gotof_58, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_59, &parser::gotof_59, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_60, &parser::gotof_60, value);
            return false;
        case regexp_token_symbol_quote:
            return call_0_0(25, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_42(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 27: return push_stack(&parser::state_63, &parser::gotof_63, v);
        default: assert(0); return false;
        }
    }

    bool state_42(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
            // shift
            push_stack(&parser::state_43, &parser::gotof_43, value);
            return false;
        case regexp_token_symbol_star:
            // shift
            push_stack(&parser::state_44, &parser::gotof_44, value);
            return false;
        case regexp_token_symbol_plus:
            // shift
            push_stack(&parser::state_45, &parser::gotof_45, value);
            return false;
        case regexp_token_symbol_question:
            // shift
            push_stack(&parser::state_46, &parser::gotof_46, value);
            return false;
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_47, &parser::gotof_47, value);
            return false;
        case regexp_token_symbol_right_pare:
            // shift
            push_stack(&parser::state_48, &parser::gotof_48, value);
            return false;
        case regexp_token_symbol_left_brace:
            // shift
            push_stack(&parser::state_49, &parser::gotof_49, value);
            return false;
        case regexp_token_symbol_right_brace:
            // shift
            push_stack(&parser::state_50, &parser::gotof_50, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_51, &parser::gotof_51, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_52, &parser::gotof_52, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_38, &parser::gotof_38, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_53, &parser::gotof_53, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_54, &parser::gotof_54, value);
            return false;
        case regexp_token_symbol_set_right_bracket:
            // shift
            push_stack(&parser::state_55, &parser::gotof_55, value);
            return false;
        case regexp_token_symbol_minus:
            // shift
            push_stack(&parser::state_56, &parser::gotof_56, value);
            return false;
        case regexp_token_symbol_comma:
            // shift
            push_stack(&parser::state_57, &parser::gotof_57, value);
            return false;
        case regexp_token_symbol_colon:
            // shift
            push_stack(&parser::state_58, &parser::gotof_58, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_59, &parser::gotof_59, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_60, &parser::gotof_60, value);
            return false;
        case regexp_token_symbol_quote:
            return call_0_identity(25, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_43(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_43(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_44(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_44(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_45(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_45(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_46(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_46(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_47(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_47(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_48(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_48(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_49(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_49(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_50(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_50(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_51(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_51(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_52(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_52(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_53(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_53(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_54(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_54(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_55(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_55(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_56(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_56(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_57(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_57(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_58(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_58(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_59(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_59(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_60(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_60(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char(27, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_61(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_61(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_1_make_char(27, 2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_62(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_62(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char_seq(26, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_63(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_63(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_1_make_char_seq(26, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_64(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_108, &parser::gotof_108, v);
        case 2: return push_stack(&parser::state_70, &parser::gotof_70, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 5: return push_stack(&parser::state_68, &parser::gotof_68, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 6: return push_stack(&parser::state_110, &parser::gotof_110, v);
        case 8: return push_stack(&parser::state_111, &parser::gotof_111, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 13: return push_stack(&parser::state_109, &parser::gotof_109, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_64(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_65(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_108, &parser::gotof_108, v);
        case 2: return push_stack(&parser::state_76, &parser::gotof_76, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 5: return push_stack(&parser::state_68, &parser::gotof_68, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 6: return push_stack(&parser::state_110, &parser::gotof_110, v);
        case 8: return push_stack(&parser::state_111, &parser::gotof_111, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 13: return push_stack(&parser::state_109, &parser::gotof_109, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_65(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_66(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_108, &parser::gotof_108, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 5: return push_stack(&parser::state_67, &parser::gotof_67, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 13: return push_stack(&parser::state_109, &parser::gotof_109, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_66(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_67(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_107, &parser::gotof_107, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_67(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_0:
            return call_0_make_union(8, 3, 0, 2, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_68(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 4: return push_stack(&parser::state_96, &parser::gotof_96, v);
        case 0: return push_stack(&parser::state_106, &parser::gotof_106, v);
        case 1: return push_stack(&parser::state_107, &parser::gotof_107, v);
        case 3: return push_stack(&parser::state_91, &parser::gotof_91, v);
        case 7: return push_stack(&parser::state_94, &parser::gotof_94, v);
        case 10: return push_stack(&parser::state_104, &parser::gotof_104, v);
        case 11: return push_stack(&parser::state_92, &parser::gotof_92, v);
        case 12: return push_stack(&parser::state_105, &parser::gotof_105, v);
        case 14: return push_stack(&parser::state_103, &parser::gotof_103, v);
        case 16: return push_stack(&parser::state_97, &parser::gotof_97, v);
        case 22: return push_stack(&parser::state_95, &parser::gotof_95, v);
        case 23: return push_stack(&parser::state_93, &parser::gotof_93, v);
        case 24: return push_stack(&parser::state_90, &parser::gotof_90, v);
        default: assert(0); return false;
        }
    }

    bool state_68(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_left_pare:
            // shift
            push_stack(&parser::state_65, &parser::gotof_65, value);
            return false;
        case regexp_token_symbol_dot:
            // shift
            push_stack(&parser::state_71, &parser::gotof_71, value);
            return false;
        case regexp_token_symbol_eos:
            // shift
            push_stack(&parser::state_69, &parser::gotof_69, value);
            return false;
        case regexp_token_symbol_backslash:
            // shift
            push_stack(&parser::state_17, &parser::gotof_17, value);
            return false;
        case regexp_token_symbol_set_left_bracket:
            // shift
            push_stack(&parser::state_10, &parser::gotof_10, value);
            return false;
        case regexp_token_symbol_hat:
            // shift
            push_stack(&parser::state_64, &parser::gotof_64, value);
            return false;
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_41, &parser::gotof_41, value);
            return false;
        case regexp_token_symbol_any_non_metacharacter:
            // shift
            push_stack(&parser::state_78, &parser::gotof_78, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_79, &parser::gotof_79, value);
            return false;
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_0:
            return call_0_identity(6, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_69(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_69(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_eos(24, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_70(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_70(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_after_nline(3, 2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_71(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_71(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_any(11, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_72(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_72(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_quote:
            // shift
            push_stack(&parser::state_73, &parser::gotof_73, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_73(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_73(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_str(23, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_74(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_74(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_set_right_bracket:
            // shift
            push_stack(&parser::state_75, &parser::gotof_75, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_75(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_75(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_set_or_class(7, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_76(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_76(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_right_pare:
            // shift
            push_stack(&parser::state_77, &parser::gotof_77, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_77(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_77(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_group(22, 3, 1, 0, 2);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_78(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_78(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_char(4, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_79(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_79(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_char(4, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_80(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_80(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_set_right_bracket:
        case regexp_token_symbol_minus:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_1_make_char(4, 2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_81(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_81(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_0_make_char_seq(21, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_82(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_82(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_colon:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
            return call_1_make_char_seq(21, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_83(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 15: return push_stack(&parser::state_84, &parser::gotof_84, v);
        case 20: return push_stack(&parser::state_86, &parser::gotof_86, v);
        default: assert(0); return false;
        }
    }

    bool state_83(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_88, &parser::gotof_88, value);
            return false;
        case regexp_token_symbol_right_brace:
            return call_0_0(20, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_84(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_84(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_89, &parser::gotof_89, value);
            return false;
        case regexp_token_symbol_right_brace:
            return call_0_identity(20, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_85(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 19: return push_stack(&parser::state_98, &parser::gotof_98, v);
        default: assert(0); return false;
        }
    }

    bool state_85(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_comma:
            // shift
            push_stack(&parser::state_83, &parser::gotof_83, value);
            return false;
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_89, &parser::gotof_89, value);
            return false;
        case regexp_token_symbol_right_brace:
            return call_0_0(19, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_86(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_86(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_right_brace:
            return call_0_make_m(19, 2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_87(int nonterminal_index, const value_type& v)
    {
        switch(nonterminal_index){
        case 15: return push_stack(&parser::state_85, &parser::gotof_85, v);
        default: assert(0); return false;
        }
    }

    bool state_87(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_number:
            // shift
            push_stack(&parser::state_88, &parser::gotof_88, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_88(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_88(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_number:
            return call_0_make_char_seq(15, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_89(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_89(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_right_brace:
        case regexp_token_symbol_comma:
        case regexp_token_symbol_number:
            return call_1_make_char_seq(15, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_90(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_90(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_91(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_91(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_92(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_92(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_93(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_93(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_94(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_94(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_95(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_95(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_96(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_96(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_elementary_regexp(16, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_97(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_97(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_star:
            // shift
            push_stack(&parser::state_102, &parser::gotof_102, value);
            return false;
        case regexp_token_symbol_plus:
            // shift
            push_stack(&parser::state_101, &parser::gotof_101, value);
            return false;
        case regexp_token_symbol_question:
            // shift
            push_stack(&parser::state_100, &parser::gotof_100, value);
            return false;
        case regexp_token_symbol_left_brace:
            // shift
            push_stack(&parser::state_87, &parser::gotof_87, value);
            return false;
        case regexp_token_symbol_or:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(1, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_98(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_98(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_right_brace:
            // shift
            push_stack(&parser::state_99, &parser::gotof_99, value);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_99(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_99(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_n_to_m(14, 5, 0, 2, 3, 1, 4);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_100(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_100(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_one_or_zero(10, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_101(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_101(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_kleene_plus(12, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_102(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_102(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_kleene(0, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_103(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_103(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(1, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_104(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_104(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(1, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_105(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_105(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(1, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_106(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_106(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(1, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_107(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_107(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_make_concat(13, 2, 0, 1);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_108(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_108(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(5, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_109(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_109(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(5, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_110(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_110(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
            // shift
            push_stack(&parser::state_66, &parser::gotof_66, value);
            return false;
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(2, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_111(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_111(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_symbol_or:
        case regexp_token_symbol_star:
        case regexp_token_symbol_plus:
        case regexp_token_symbol_question:
        case regexp_token_symbol_left_pare:
        case regexp_token_symbol_right_pare:
        case regexp_token_symbol_left_brace:
        case regexp_token_symbol_dot:
        case regexp_token_symbol_eos:
        case regexp_token_symbol_backslash:
        case regexp_token_symbol_set_left_bracket:
        case regexp_token_symbol_hat:
        case regexp_token_symbol_quote:
        case regexp_token_symbol_any_non_metacharacter:
        case regexp_token_symbol_number:
        case regexp_token_0:
            return call_0_identity(6, 1, 0);
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

    bool gotof_112(int nonterminal_index, const value_type& v)
    {
        assert(0);
        return true;
    }

    bool state_112(token_type token, const value_type& value)
    {
        switch(token){
        case regexp_token_0:
            // accept
            // run_semantic_action();
            accepted_ = true;
            accepted_value_ = get_arg(1, 0);
            return false;
        default:
            sa_.syntax_error();
            error_ = true;
            return false;
        }
    }

};

} // namespace regexp_parser

#endif // REGEXP_PARSER_HPP_
