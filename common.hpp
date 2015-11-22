#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <string>

struct eos_functor{
    inline int operator ()() const{
        return std::numeric_limits<int>::max();
    }
};

struct error_token_functor{
    inline int operator ()() const{
        return std::numeric_limits<int>::max() - 1;
    }
};

struct whitespace_functor{
    inline int operator ()() const{
        return std::numeric_limits<int>::max() - 2;
    }
};

struct dummy_functor{
    inline int operator ()() const{
        return std::numeric_limits<int>::max() - 3;
    }
};

struct epsilon_functor{
    inline int operator ()() const{
        return 0;
    }
};

struct indent_type{
    int n = 0;
    std::string operator()() const{
        std::string ret;
        for(int i = 0; i < n; ++i){
            ret += "    ";
        }
        return ret;
    }

    indent_type &operator ++(){
        ++n;
        return *this;
    }

    indent_type &operator --(){
        --n;
        return *this;
    }
};

template<class String>
class string_iter_pair{
private:
    typename String::const_iterator begin_, end_;

public:
    using value_type = typename String::const_iterator::value_type;
    using iterator_type = typename String::const_iterator;
    string_iter_pair() = default;
    string_iter_pair(iterator_type begin_, iterator_type end) : begin_(begin_), end_(end){}
    iterator_type begin(){
        return begin_;
    }

    ~string_iter_pair() = default;

    string_iter_pair &operator =(string_iter_pair const &other){
        begin_ = other.begin_;
        end_ = other.end_;
        return *this;
    }


    bool operator <(string_iter_pair const &other) const{
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    bool operator ==(char const *str) const{
        iterator_type iter = begin_;
        for(; iter != end_ && *str != '\0'; ++iter, ++str){
            if(*iter != *str){ return false; }
        }
        return iter == end_ && *str == '\0';
    }

    iterator_type end(){
        return end_;
    }

    iterator_type begin() const{
        return begin_;
    }

    iterator_type end() const{
        return end_;
    }

    iterator_type cbegin() const{
        return begin_;
    }

    iterator_type cend() const{
        return end_;
    }

    bool empty() const{
        return begin_ == end_;
    }

    std::string to_str() const{
        return std::string(begin_, end_);
    }
};

template<class String>
class symbol_manager_type{
private:
    int terminal_counter = 1, nonterminal_counter = -1;
    std::map<String, int> str_to_term;
    std::map<int, String> term_to_str;

public:
    const std::map<String, int> &ref_str_to_term = str_to_term;
    const std::map<int, String> &ref_term_to_str = term_to_str;

    void register_special_term(String const &str, int id){
        str_to_term.insert(std::make_pair(str, id));
        term_to_str.insert(std::make_pair(id, str));
    }

    int set_terminal(String const &str){
        auto iter = str_to_term.find(str);
        if(iter != str_to_term.end()){
            return iter->second;
        }else{
            str_to_term.insert(std::make_pair(str, terminal_counter));
            term_to_str.insert(std::make_pair(terminal_counter, str));
            return terminal_counter++;
        }
    }

    int set_nonterminal(String const &str){
        auto iter = str_to_term.find(str);
        if(iter != str_to_term.end()){
            return iter->second;
        }else{
            str_to_term.insert(std::make_pair(str, nonterminal_counter));
            term_to_str.insert(std::make_pair(nonterminal_counter, str));
            return nonterminal_counter--;
        }
    }

    int get(String const &str) const{
        return str_to_term.find(str)->second;
    }

    String const &to_str(int t) const{
        return term_to_str.find(t)->second;
    }
};

#endif // COMMON_HPP

