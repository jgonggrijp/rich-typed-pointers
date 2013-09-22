#include <rich_typed_ptr.hpp>
namespace rtp = rich_typed_ptr;

#include <iostream>
#include <utility>
#include <algorithm>
#include <cstddef>
using namespace std;

template <class T> class list_iterator;
template <class T> class list;

template <class T>
class list_node {

public:
    using data_ptr = rtp::data_ptr<list_node>;
    using weak_ptr = rtp::weak_ptr<list_node>;
    using iterator = list_iterator<T>;

    list_node (const T & value, data_ptr && n, weak_ptr p) :
                                                    data(value),
                                                    next(forward<data_ptr>(n)),
                                                    prev(p) {
    }

    void append (const T & value) {
        data_ptr insertion = rtp::make<list_node>(value, move(next), prev->next);
        swap(insertion, next);
        if (next->next != nullptr) next->next->prev = next;
    }
    void prepend (const T & value) {
        prev->next = rtp::make<list_node>(value, move(prev->next), prev);
        prev = prev->next;
    }
    void remove_next ( ) {
        auto excision = move(next);
        next = move(excision->next);
        if (next != nullptr) next->prev = excision->prev;
    }
    void remove ( ) { prev->remove_next(); }

    friend list_iterator<T>;
    friend list<T>;

private:
    T        data;
    data_ptr next;
    weak_ptr prev;

};

template <class T>
class list_iterator {

private:
    using node = list_node<T>;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using pointer = T*;

    list_iterator (typename node::weak_ptr p) : position(p) { }

    T & operator* ( ) { return position->data; }
    T * operator-> ( ) { return &(position->data); }

    list_iterator & operator++ ( )   {
        position = position->next;
        return *this;
    }
    list_iterator   operator++ (int) {
        auto previous = *this;
        ++(*this);
        return previous;
    }

    friend bool operator== (list_iterator left, list_iterator right) {
        return left.position == right.position;
    }

    friend bool operator!= (list_iterator left, list_iterator right) {
        return left.position != right.position;
    }

    friend list<T>;

private:
    typename node::weak_ptr position;

};

template <class T>
class list {

private:
    using node = list_node<T>;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using pointer = T*;
    using iterator = list_iterator<T>;

    iterator begin ( ) { return rtp::weak<node>(first); }
    iterator end ( ) { return (last != nullptr) ? last->next : last; }

    T & front ( ) { return first->data; }
    T & back ( ) { return last->data; }

    void push_front (const T & value) {
        if (first != nullptr) {
            first = rtp::make<node>(value, move(first), first->prev);
            if (first->next != nullptr) first->next->prev = first;
        }
        else inaugurate(value);
    }
    void push_back (const T & value) {
        if      (last == nullptr) inaugurate(value);
        else if (last == first) {
            last->next = rtp::make<node>(value, move(last->next), last);
        } else {
            last->append(value);
            last = last->next;
        }
    }
    void insert (iterator pos, const T & value) {
        if      (pos.position == first)     push_front(value);
        else if (pos.position != nullptr)   pos.position->prepend(value);
        else                                push_back(value);
    }

    void pop_front ( ) {
        if (first != nullptr) {
            auto trash = move(first);
            first = move(trash->next);
            if   (first != nullptr) first->prev = trash->prev;
            else                    last = first;
        }
    }
    void pop_back ( ) {
        if      (last == first) pop_front();
        else if (last != nullptr) {
            auto trash = move(last->prev->next);
            last = trash->prev;
        }
    }
    void erase (iterator pos) {
        if      (pos.position == first)     pop_front();
        else if (pos.position != nullptr)   pos.position->remove();
    }

    bool empty ( ) { return first == nullptr; }
    void clear ( ) {
        auto trash = move(first);
        last = first;
    }

private:
    typename node::data_ptr first = nullptr;
    typename node::weak_ptr last = first;

    void inaugurate (const T & value) {
        first = rtp::make<node>(value, move(first), first);
        last = first;
    }

};

int main ( ) {
    list<int> test;
    test.push_back(2);
    test.push_back(3);
    test.push_front(4);
    auto pos2 = find(begin(test), end(test), 2);
    test.insert(pos2, 5);
    copy(begin(test), end(test), ostream_iterator<int>(cout, " "));  // 4 5 2 3
    cout << endl << test.front() << ' ' << test.back() << endl;  // 4 3
    test.pop_front();
    test.erase(pos2);
    test.pop_back();
    test.push_back(6);
    test.clear();
    cout << test.empty() << endl;  // true
    return 0;  // memory neatly reclaimed here
}
