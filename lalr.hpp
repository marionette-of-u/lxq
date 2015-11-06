#ifndef LALR_HPP_
#define LALR_HPP_

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

template<class Term>
struct default_epsilon_functor{
    Term operator ()() const{
        return Term();
    }
};

template<class Term>
struct default_is_terminal_symbol_functor{
    template<class G>
    bool operator ()(Term const &x, G const &g) const{
        return g.find(x) == g.end();
    }
};

template<
    class SemanticType,
    class Term,
    class EOSFunctor,
    class DummyFunctor,
    class IsTerminalSymbolFunctor = default_is_terminal_symbol_functor<Term>,
    class EpsilonFunctor = default_epsilon_functor<Term>
>
class lalr{
public:
    using semantic_type = SemanticType;
    using term = Term;
    using eos_functor = EOSFunctor;
    using dummy_functor = DummyFunctor;
    using is_terminal_symbol_functor = IsTerminalSymbolFunctor;
    using epsilon_functor = EpsilonFunctor;
    using term_set = std::set<term>;
    using first_set_type = std::map<term, term_set>;

    enum class linkdir{
        nonassoc,
        left,
        right
    };

    struct symbol_data_type{
        linkdir dir = linkdir::nonassoc;
        std::size_t priority = 0;
    };

    using symbol_data_map = std::map<term, symbol_data_type>;

    class term_sequence : public std::vector<term>{
    public:
        term_sequence() : std::vector<term>(), semantic_data(){}
        term_sequence(term_sequence const &other) : std::vector<term>(other), semantic_data(other.semantic_data){}
        term_sequence(std::initializer_list<term> list, semantic_type const &s) : std::vector<term>(list), semantic_data(s){}
        semantic_type semantic_data;
        term tag = epsilon_functor()();
    };

    struct item{
        term lhs;
        term_sequence rhs;
        std::size_t pos;
        mutable term_set lookahead;

        bool is_over() const{
            return pos >= rhs.size();
        }

        term const &curr() const{
            return rhs[pos];
        }

        bool equal(item const &other) const{
            return lhs == other.lhs && rhs == other.rhs && pos == other.pos && lookahead == other.lookahead;
        }
    };

    struct rule_rhs_comparetor{
        bool operator ()(term_sequence const &x, term_sequence const &y) const{
            for(std::size_t i = 0; i < x.size() && i < y.size(); ++i){
                if(x[i] < y[i]){
                    return true;
                }else if(x[i] > y[i]){
                    return false;
                }
            }
            return x.size() < y.size();
        }
    };

    struct item_comparetor{
        bool operator ()(item const &x, item const &y) const{
            if(x.lhs < y.lhs){
                return true;
            }else if(x.lhs > y.lhs){
                return false;
            }
            if(x.pos < y.pos){
                return true;
            }else if(x.pos > y.pos){
                return false;
            }
            if(rule_rhs_comparetor()(x.rhs, y.rhs)){
                return true;
            }
            return false;
        }
    };

    struct exteded_item_comparetor{
        bool operator ()(item const &x, item const &y) const{
            if(x.lhs < y.lhs){
                return true;
            }else if(x.lhs > y.lhs){
                return false;
            }
            if(x.pos < y.pos){
                return true;
            }else if(x.pos > y.pos){
                return false;
            }
            if(rule_rhs_comparetor()(x.rhs, y.rhs)){
                return true;
            }else if(rule_rhs_comparetor()(y.rhs, x.rhs)){
                return false;
            }
            return x.lookahead < y.lookahead;
        }
    };

    using rule_rhs = std::set<term_sequence, rule_rhs_comparetor>;
    using grammar = std::map<term, rule_rhs>;

    class items : public std::set<item, item_comparetor>{
    public:
        using base_type = std::set<item, item_comparetor>;
        using goto_map_type = std::map<term, items const*>;

        items const mutable *mirror;
        goto_map_type mutable goto_map;

        items() : base_type(), mirror(nullptr), goto_map(){}
        items(items const &other) : base_type(other), mirror(other.mirror), goto_map(other.goto_map){}
        items(items &&other) : base_type(other), mirror(std::move(other.mirror)), goto_map(std::move(other.goto_map)){}
        items(std::initializer_list<item> list) : base_type(list), mirror(nullptr), goto_map(){}

        items &operator =(items const &other){
            base_type::operator =(other);
            mirror = other.mirror;
            goto_map.operator =(other.goto_map);
            return *this;
        }

        items &operator =(items &&other){
            base_type::operator =(other);
            mirror = std::move(other.mirror);
            goto_map.operator =(std::move(other.goto_map));
            return *this;
        }

        std::pair<typename base_type::iterator, bool> insert(item const &i){
            std::pair<typename base_type::iterator, bool> p = base_type::insert(i);
            if(!p.second){
                p.first->lookahead.insert(i.lookahead.begin(), i.lookahead.end());
            }
            return p;
        }
    };

    struct items_comparetor{
        bool operator ()(items const &x, items const &y) const{
            auto xter = x.begin(), yter = y.begin();
            for(; xter != x.end() && yter != y.end(); ++xter, ++yter){
                if(exteded_item_comparetor()(*xter, *yter)){
                    return true;
                }else if(exteded_item_comparetor()(*yter, *xter)){
                    return false;
                }
            }
            return x.size() < y.size();
        }
    };

    struct items_ptr_comparetor{
        bool operator ()(items const *x, items const *y) const{
            auto xter = x->begin(), yter = y->begin();
            for(; xter != x->end() && yter != y->end(); ++xter, ++yter){
                if(exteded_item_comparetor()(*xter, *yter)){
                    return true;
                }else if(exteded_item_comparetor()(*yter, *xter)){
                    return false;
                }
            }
            return x->size() < y->size();
        }
    };

    using states = std::set<items, items_comparetor>;

    struct limited_items_comparetor{
        bool operator ()(items const *x, items const *y) const{
            auto xter = x->begin(), yter = y->begin();
            for(; xter != x->end() && yter != y->end(); ++xter, ++yter){
                if(item_comparetor()(*xter, *yter)){
                    return true;
                }else if(item_comparetor()(*yter, *xter)){
                    return false;
                }
            }
            return x->size() < y->size();
        }
    };

    using lr0_kernel_states = std::set<items const*, limited_items_comparetor>;

    struct lr_parsing_table_item{
        enum class enum_action{
            shift,
            reduce,
            accept
        };

        enum_action action;
        std::size_t num;
        item const *item_ptr = nullptr;

        bool operator ==(lr_parsing_table_item const &other) const{
            return action == other.action && num == other.num;
        }

        bool operator !=(lr_parsing_table_item const &other) const{
            return !(*this == other);
        }
    };

    struct lr_parsing_table_item_comparetor{
        bool operator ()(lr_parsing_table_item const &lhs, lr_parsing_table_item const &rhs) const{
            if(lhs.action < rhs.action){
                return true;
            }else if(lhs.action > rhs.action){
                return false;
            }
            return lhs.num < rhs.num;
        }
    };

    struct lr_conflict{
        lr_parsing_table_item lhs, rhs;
    };

    struct lr_conflict_comparetor{
        bool operator ()(lr_conflict const &lhs, lr_conflict const &rhs) const{
            if(lr_parsing_table_item_comparetor()(lhs.lhs, rhs.lhs)){
                return true;
            }else if(lr_parsing_table_item_comparetor()(rhs.lhs, lhs.lhs)){
                return false;
            }
            return lr_parsing_table_item_comparetor()(lhs.rhs, rhs.rhs);
        }
    };

    using lr_parsing_table = std::map<std::size_t, std::map<term, lr_parsing_table_item>>;
    using lr_goto_table = std::map<std::size_t, std::map<term, std::size_t>>;
    using state_to_num = std::map<items const*, std::size_t, limited_items_comparetor>;
    using num_to_state = std::map<std::size_t, items const*>;
    using rule_to_num = std::map<term_sequence const*, std::size_t>;
    using num_to_rule = std::map<std::size_t, std::pair<term, term_sequence const*>>;
    using lr_conflict_set = std::set<lr_conflict, lr_conflict_comparetor>;

    struct make_result{
        std::size_t first;
        state_to_num s2n;
        num_to_state n2s;
        rule_to_num r2n;
        num_to_rule n2r;
        lr_parsing_table parsing_table;
        lr_goto_table goto_table;
        lr_conflict_set conflict_set;
    };

    static bool states_comparetor(states const &x, states const &y){
        auto xter = x.begin(), yter = y.begin();
        for(; xter != x.end() && yter != y.end(); ++xter, ++yter){
            if(items_comparetor()(*xter, *yter) || items_comparetor()(*yter, *xter)){
                return false;
            }
        }
        return x.size() == y.size();
    }


    template<class Set>
    static Set union_set(Set const &x, Set const &y){
        Set i = x;
        for(auto iter = y.begin(); iter != y.end(); ++iter){
            i.insert(*iter);
        }
        return i;
    }

    template<class Set>
    static Set inter_set(Set const &x, Set const &y){
        Set i;
        for(auto iter = x.begin(); iter != x.end(); ++iter){
            if(y.find(*iter) != y.end()){
                i.insert(*iter);
            }
        }
        return i;
    }

    static term_set make_terminal_symbol_set(grammar const &g){
        term_set s;
        for(auto iter = g.begin(); iter != g.end(); ++iter){
            for(auto jter = iter->second.begin(); jter != iter->second.end(); ++jter){
                for(auto kter = jter->begin(); kter != jter->end(); ++kter){
                    if(is_terminal_symbol_functor()(*kter, g)){
                        s.insert(*kter);
                    }
                }
            }
        }
        return s;
    }

    mutable std::map<term, term_set> first_set_cache;
    term_set const &first_set(grammar const &g, term const &x){
        std::set<term> s;
        auto p = first_set_cache.insert(std::make_pair(x, term_set()));
        if(p.second){
            p.first->second = first_set_impl(g, x, s);
        }
        return p.first->second;
    }

    static term_set first_set_impl(grammar const &g, term const &x, std::set<term> &scanned){
        term_set r;
        if(is_terminal_symbol_functor()(x, g)){
            r.insert(x);
            return r;
        }
        if(scanned.find(x) != scanned.end()){
            return r;
        }
        scanned.insert(x);
        rule_rhs const &rhs(g.find(x)->second);
        auto iter = rhs.begin();
        for(; iter != rhs.end(); ++iter){
            if(iter->empty()){
                r.insert(epsilon_functor()());
                continue;
            }
            auto jter = iter->begin();
            bool epsilon = true;
            for(; epsilon && jter != iter->end(); ++jter){
                term_set s = first_set_impl(g, *jter, scanned);
                epsilon = s.find(epsilon_functor()()) != s.end();
                if(epsilon){
                    s.erase(epsilon_functor()());
                }
                r = union_set(r, s);
            }
            if(epsilon && jter == iter->end()){
                r.insert(epsilon_functor()());
            }
        }
        return r;
    }

    std::map<term, term_set> follow_set;
    void make_follow_set(grammar const &g, term const &s){
        follow_set[s].insert(eos_functor()());
        std::size_t size = 0, psize;
        do{
            psize = size;
            for(std::pair<term, rule_rhs> const &p : g){
                for(term_sequence const &seq : p.second){
                    if(seq.size() < 2){ continue; }
                    for(auto iter = seq.rbegin() + 1; iter != seq.rend() - 1; ++iter){
                        term const &b = *iter, &beta = *(iter - 1);
                        if(is_terminal_symbol_functor()(b, g)){ continue; }
                        term_set &b_set = follow_set[b];
                        std::size_t b_size = b_set.size();
                        for(auto jter = iter; ; ){
                            if(jter == seq.rbegin()){ break; }
                            --jter;
                            term_set const &first_beta(first_set(g, *jter));
                            b_set.insert(first_beta.begin(), first_beta.end());
                            if(b_set.find(epsilon_functor()()) != b_set.end()){
                                b_set.erase(epsilon_functor()());
                            }
                            size += b_set.size() - b_size;
                            b_size = b_set.size();
                        }
                    }
                }
            }
            for(std::pair<term, rule_rhs> const &p : g){
                for(term_sequence const &seq : p.second){
                    if(seq.empty()){ continue; }
                    term_set const &a_set = follow_set[p.first];
                    if(!is_terminal_symbol_functor()(seq.back(), g)){
                        term const &b = seq.back();
                        term_set &b_set = follow_set[b];
                        std::size_t b_size = b_set.size();
                        b_set.insert(a_set.begin(), a_set.end());
                        size += b_set.size() - b_size;
                    }
                    if(seq.size() == 1){ continue; }
                    for(auto iter = seq.rbegin() + 1; iter != seq.rend() - 1; ++iter){
                        term const &b = *iter, &beta = *(iter - 1);
                        if(is_terminal_symbol_functor()(b, g)){ continue; }
                        term_set &b_set = follow_set[b];
                        std::size_t b_size = b_set.size();
                        for(auto jter = iter; ; ){
                            if(jter == seq.rbegin()){ break; }
                            --jter;
                            term_set const &first_beta(first_set(g, *jter));
                            bool epsilon = first_beta.find(epsilon_functor()()) != first_beta.end();
                            if(epsilon){
                                b_set.insert(a_set.begin(), a_set.end());
                                size += b_set.size() - b_size;
                                b_size = b_set.size();
                            }
                        }
                    }
                }
            }
        }while(size != psize);
    }

    static items lr0_closure(grammar const &g, items j){
        std::size_t s;
        do{
            s = j.size();
            for(auto iter = j.begin(); iter != j.end(); ++iter){
                if(iter->is_over() || is_terminal_symbol_functor()(iter->curr(), g)){
                    continue;
                }
                term const &b = iter->curr();
                rule_rhs const &rule_b = g.find(b)->second;
                for(auto rhs_iter = rule_b.begin(); rhs_iter != rule_b.end(); ++rhs_iter){
                    item n_item;
                    n_item.pos = 0;
                    n_item.lhs = b;
                    n_item.rhs = *rhs_iter;
                    if(j.find(n_item) == j.end()){
                        j.insert(n_item);
                    }
                }
            }
        }while(s != j.size());
        return j;
    }

    static items lr0_goto(grammar const &g, items const &i, term const &x){
        items r;
        for(auto iter = i.begin(); iter != i.end(); ++iter){
            item const &current_item(*iter);
            if(current_item.is_over()){
                continue;
            }
            if(current_item.curr() != x){
                continue;
            }
            item next_item = current_item;
            ++next_item.pos;
            r.insert(next_item);
        }
        return lr0_closure(g, r);
    }

    static states lr0_items(grammar const &g, term_set const &terminal_symbol_set, item const &start){
        states c;
        {
            items init;
            init.insert(start);
            c.insert(lr0_closure(g, init));
        }
        std::size_t size;
        do{
            size = c.size();
            for(auto iter = c.begin(); iter != c.end(); ++iter){
                items const &i(*iter);
                auto lambda = [&](term const &x){
                    items n_items = lr0_goto(g, i, x);
                    if(!n_items.empty() && c.find(n_items) == c.end()){
                        c.insert(n_items);
                    }
                };
                for(auto gter = g.begin(); gter != g.end(); ++gter){
                    lambda(gter->first);
                }
                for(auto xter = terminal_symbol_set.begin(); xter != terminal_symbol_set.end(); ++xter){
                    lambda(*xter);
                }
            }
        }while(size != c.size());
        return c;
    }

    items lr1_closure(grammar const &g, items i){
        std::size_t s;
        do{
            s = i.size();
            for(auto iter = i.begin(); iter != i.end(); ++iter){
                item const &i_item(*iter);
                if(i_item.is_over()){
                    continue;
                }
                if(is_terminal_symbol_functor()(i_item.curr(), g)){
                    continue;
                }
                rule_rhs const &rule(g.find(i_item.curr())->second);
                for(auto jter = rule.begin(); jter != rule.end(); ++jter){
                    term_set first_beta_a;
                    if(i_item.pos + 1 < i_item.rhs.size()){
                        for(std::size_t n = i_item.pos + 1; n < i_item.rhs.size(); ++n){
                            term_set const &f = first_set(g, i_item.rhs[n]);
                            bool epsilon = f.find(epsilon_functor()()) != f.end();
                            for(term const &fe : f){
                                first_beta_a.insert(fe);
                            }
                            if(!epsilon){
                                break;
                            }
                        }
                    }
                    if(first_beta_a.empty()){
                        first_beta_a = i_item.lookahead;
                    }else if(first_beta_a.find(epsilon_functor()()) != first_beta_a.end()){
                        first_beta_a.erase(epsilon_functor()());
                        first_beta_a.insert(i_item.lookahead.begin(), i_item.lookahead.end());
                    }
                    item t;
                    t.lhs = i_item.curr();
                    t.rhs = *jter;
                    t.pos = 0;
                    t.lookahead = first_beta_a;
                    i.insert(t);
                }
            }
        }while(s != i.size());
        return i;
    }

    items lr1_goto(grammar const &g, items const &i, term const &x){
        items j;
        for(auto iter = i.begin(); iter != i.end(); ++iter){
            if(iter->is_over()){
                continue;
            }
            if(iter->curr() == x){
                item t = *iter;
                ++t.pos;
                j.insert(t);
            }
        }
        return lr1_closure(g, j);
    }

    static states lr1_items(grammar const &g, term_set const &terminal_symbol_set, item const &start){
        states c;
        {
            items init;
            init.insert(start);
            c.insert(lr1_closure(g, init));
        }
        std::size_t s;
        do{
            s = c.size();
            for(auto iter = c.begin(); iter != c.end(); ++iter){
                for(auto jter = g.begin(); jter != g.end(); ++jter){
                    items n = lr1_goto(g, *iter, jter->first);
                    if(!n.empty() && c.find(n) == c.end()){
                        c.insert(n);
                    }
                }
                for(auto jter = terminal_symbol_set.begin(); jter != terminal_symbol_set.end(); ++jter){
                    items n = lr1_goto(g, *iter, *jter);
                    if(!n.empty() && c.find(n) == c.end()){
                        c.insert(n);
                    }
                }
            }
        }while(s != c.size());
        return c;
    }

    static items kernel_filter(items i, item const &start){
        for(auto iter = i.begin(); iter != i.end(); ){
            if(iter->equal(start)){
                ++iter;
                continue;
            }
            if(iter->pos == 0){
                iter = i.erase(iter);
            }else{
                ++iter;
            }
        }
        return i;
    }

    static void lr0_kernel_items(grammar const &g, states &c_prime, states &c, typename states::iterator &first_state, term_set const &terminal_symbol_set, item const &start){
        c_prime = lr0_items(g, terminal_symbol_set, start);
        for(items const &i : c_prime){
            std::pair<states::iterator, bool> p = c.insert(kernel_filter(i, start));
            p.first->mirror = &i;
            i.mirror = &*p.first;
            if(
                p.first->size() == 1 &&
                p.first->begin()->lhs == -1 &&
                p.first->begin()->rhs.size() == 1 &&
                *p.first->begin()->rhs.begin() == -2
            ){
                first_state = p.first;
            }
        }
    }

    static void make_goto_map(grammar const &g, term_set const &h, states const &c_prime, states const &c, item const &start){
        term_set k;
        for(auto const &r : g){
            k.insert(r.first);
        }
        for(items const &s : c_prime){
            auto f = [&](term_set const &ts){
                for(term const &t : ts){
                    items n_goto = kernel_filter(lr0_goto(g, s, t), start);
                    if(n_goto.empty()){
                        continue;
                    }
                    s.mirror->goto_map[t] = &*c.find(n_goto);
                }
            };
            f(h);
            f(k);
        }
    }

    void completion_lookahead(grammar const &g, states &s, typename states::iterator first_state, item const &start){
        if(first_state != s.end()){
            first_state->begin()->lookahead.insert(eos_functor()());
        }
        std::map<item const*, std::set<item const*>> prop_map;
        lr0_kernel_states s_prime;
        for(auto const &k : s){
            s_prime.insert(&k);
        }
        for(items const &k : s){
            for(item const &i : k){
                std::set<item const*> &prop_set(prop_map[&i]);
                item i_prime = i;
                i_prime.lookahead.clear();
                i_prime.lookahead.insert(dummy_functor()());
                items j;
                j.insert(i_prime);
                j = lr1_closure(g, j);
                for(item const &b : j){
                    if(b.is_over()){
                        continue; 
                    }
                    items n_goto = kernel_filter(lr1_goto(g, k, b.curr()), start);
                    item next = b;
                    ++next.pos;
                    item const *next_b = &*k.goto_map[b.curr()]->find(next);
                    for(term const &t : b.lookahead){
                        if(t != dummy_functor()()){
                            next_b->lookahead.insert(t);
                        }else{
                            prop_set.insert(next_b);
                        }
                    }
                }
            }
        }
        bool z;
        do{
            z = false;
            for(std::pair<item const*, std::set<item const*>> const &p : prop_map){
                for(item const *dest : p.second){
                    std::size_t prev_size = dest->lookahead.size();
                    dest->lookahead.insert(p.first->lookahead.begin(), p.first->lookahead.end());
                    z = z || dest->lookahead.size() != prev_size;
                }
            }
        }while(z);
    }

    states c_closure(grammar const &g, states const &c, typename states::iterator &first_state, typename items::iterator &first_item){
        states c_prime;
        for(items const &s : c){
            items t = lr1_closure(g, s);
            if(first_state != c.end() && &s == &*first_state){
                c_prime.insert(t);
                typename states::iterator tmp_iter = c_prime.find(t);
                first_item = tmp_iter->find(*first_state->begin());
                first_state = tmp_iter;
            }else{
                c_prime.insert(std::move(t));
            }
        }
        return c_prime;
    }

    make_result make2(
        grammar const &g,
        states const &c,
        typename states::const_iterator first_state,
        item const &start,
        symbol_data_map const &symbol_data_map
    ){
        make_result result;
        lr0_kernel_states s_prime;
        rule_to_num &r2n(result.r2n);
        num_to_rule &n2r(result.n2r);
        {
            std::size_t i = 0;
            for(auto const &r : g){
                for(auto const &rr : r.second){
                    n2r[i] = std::make_pair(r.first, &rr);
                    r2n[&rr] = i;
                    ++i;
                }
            }
        }
        state_to_num &s2n(result.s2n);
        num_to_state &n2s(result.n2s);
        {
            std::size_t i = 0;
            if(first_state != c.end()){
                s_prime.insert(&*first_state);
                s2n[&*first_state] = i;
                n2s[i] = &*first_state;
                ++i;
            }
            for(auto const &s : c){
                if(first_state != c.end() && &s == &*first_state){
                    continue;
                }
                s_prime.insert(&s);
                s2n[&s] = i;
                n2s[i] = &s;
                ++i;
            }
        }
        lr_parsing_table &parsing_table(result.parsing_table);
        lr_conflict_set &conflict_set(result.conflict_set);
        auto priority_sup = [&](term_sequence const &seq){
            symbol_data_type p;
            for(auto &t : seq){
                if(!is_terminal_symbol_functor()(t, g)){ continue; }
                auto iter = symbol_data_map.find(t);
                if(iter == symbol_data_map.end()){ continue; }
                symbol_data_type const &data = iter->second;
                if(data.priority > p.priority){
                    p = data;
                }
            }
            return p;
        };
        auto insert_conflict = [&](lr_parsing_table_item const &x, lr_parsing_table_item const &y){
            conflict_set.insert(lr_conflict({ x, y }));
        };

        for(items const &state : c){
            for(item const &i : state){
                if(i.is_over()){
                    if(i.lhs == start.lhs && i.rhs == start.rhs && i.lookahead == start.lookahead){
                        // accept
                        lr_parsing_table_item a;
                        a.action = lr_parsing_table_item::enum_action::accept;
                        a.num = 0;
                        a.item_ptr = &i;
                        auto p = parsing_table[s2n[&state]].insert(std::make_pair(eos_functor()(), a));
                        if(!p.second){
                            insert_conflict(p.first->second, a);
                        }
                    }else{
                        // reduce
                        lr_parsing_table_item a;
                        a.action = lr_parsing_table_item::enum_action::reduce;
                        a.num = r2n[&*g.find(i.lhs)->second.find(i.rhs)];
                        a.item_ptr = &i;
                        for(term const &t : i.lookahead){
                            auto &parsing_table_item = parsing_table[s2n[&state]];
                            auto p = parsing_table_item.insert(std::make_pair(t, a));
                            if(!p.second){
                                lr_parsing_table_item const &other = p.first->second;
                                if(other.action == lr_parsing_table_item::enum_action::reduce){
                                    insert_conflict(other, a);
                                }else{
                                    term const &tag = i.rhs.tag, &other_tag = other.item_ptr->rhs.tag;
                                    symbol_data_type symbol_data, other_symbol_data;
                                    if(tag == epsilon_functor()()){
                                        symbol_data = priority_sup(i.rhs);
                                    }else{
                                        symbol_data = symbol_data_map.find(tag)->second;
                                    }
                                    if(other_tag == epsilon_functor()()){
                                        other_symbol_data = priority_sup(other.item_ptr->rhs);
                                    }else{
                                        other_symbol_data = symbol_data_map.find(other_tag)->second;
                                    }
                                    if(symbol_data.priority > other_symbol_data.priority){
                                        p.first->second = a;
                                    }else if(symbol_data.priority == other_symbol_data.priority){
                                        if(symbol_data.dir == linkdir::left){
                                            p.first->second = a;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }else{
                    if(is_terminal_symbol_functor()(i.curr(), g)){
                        // shift
                        lr_parsing_table_item a;
                        items n_goto = lr0_goto(g, state, i.curr());
                        a.action = lr_parsing_table_item::enum_action::shift;
                        a.num = s2n[&n_goto];
                        a.item_ptr = &i;
                        auto &parsing_table_item = parsing_table[s2n[&state]];
                        auto p = parsing_table_item.insert(std::make_pair(i.curr(), a));
                        if(!p.second){
                            lr_parsing_table_item const &other = p.first->second;
                            if(other.action == lr_parsing_table_item::enum_action::reduce){
                                symbol_data_type symbol_data, other_symbol_data;
                                term const &tag = a.item_ptr->rhs.tag;
                                auto iter = symbol_data_map.find(tag), other_iter = symbol_data_map.find(i.curr());
                                if(iter != symbol_data_map.end()){
                                    symbol_data = iter->second;
                                }else{
                                    symbol_data = priority_sup(i.rhs);
                                }
                                if(other_iter != symbol_data_map.end()){
                                    other_symbol_data = other_iter->second;
                                }
                                if(symbol_data.priority == other_symbol_data.priority){
                                    if(symbol_data.dir == linkdir::right){
                                        p.first->second = a;
                                    }
                                }
                            }
                        }
                    }else if(i.pos == 0 && i.lhs == start.lhs && i.rhs == start.rhs && i.lookahead == start.lookahead){
                        result.first = s2n[&state];
                    }
                }
            }
        }
        lr_goto_table &goto_table(result.goto_table);
        for(auto const &p : s2n){
            auto &goto_map(goto_table[p.second]);
            for(auto const &r : g){
                items n_goto = lr1_goto(g, *p.first, r.first);
                if(s2n.find(&n_goto) != s2n.end()){
                    goto_map.insert(std::make_pair(r.first, s2n[&n_goto]));
                }
            }
        }
        return result;
    }

    static std::ostream &out_term_set(std::ostream &os, term_set const &s){
        for(auto iter = s.begin(); iter != s.end(); ++iter){
            os << *iter;
            auto jter = iter;
            ++jter;
            if(jter != s.end()){
                os << ", ";
            }
        }
        return os;
    }

    static std::ostream &out_item(std::ostream &os, item const &i){
        os << i.lhs;
        os << " -> ";
        for(std::size_t count = 0; count < i.pos; ++count){
            os << i.rhs[count] << " ";
        }
        os << ".";
        for(std::size_t count = i.pos; count < i.rhs.size(); ++count){
            os << i.rhs[count] << " ";
        }
        os << " [";
        for(auto iter = i.lookahead.begin(); iter != i.lookahead.end(); ++iter){
            os << *iter << " ";
        }
        os << "]";
        return os;
    }

    static std::ostream &out_items(std::ostream &os, items const &is){
        for(auto iter = is.begin(); iter != is.end(); ++iter){
            out_item(os, *iter) << "\n";
        }
        return os;
    }

    static std::ostream &out_states(std::ostream &os, states const &c){
        std::size_t count = 0;
        for(auto iter = c.begin(); iter != c.end(); ++iter, ++count){
            os << "-------- " << count << "\n";
            out_items(os, *iter);
            os << "\n";
        }
        return os;
    }
};

#endif
