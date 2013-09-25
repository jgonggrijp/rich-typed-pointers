// NOTE: this is meant to illustrate usage of rich-typed pointers,
//       not to illustrate good C++ coding practices. ;-)

#include <rich_typed_ptr.hpp>
namespace rtp = rich_typed_ptr;

#include <iostream>
#include <utility>
#include <algorithm>
#include <cstddef>
using namespace std;

template <class T> class list_iterator;
template <class T> class list;  // (it's going to be a doubly linked one)

template <class T>
class list_node {

private:
    using data_ptr = rtp::data_ptr<list_node>;
    using weak_ptr = rtp::weak_ptr<list_node>;

    T        data;
    data_ptr next;  // a node owns the next node...
    weak_ptr prev;  // ... but not the previous

public:
    // constructor takes rvalue reference to data_prt in order to
    // take over ownership
    list_node (const T & value, data_ptr && n, weak_ptr p) :
        data(value),
        next(forward<data_ptr>(n)),  // taking over ownership with std::forward
        prev(p)                                                 { }
        // note that the constructor cannot link neighbours to itself,
        // because it doesn't have a rich-typed pointer to itself yet.

    // default dtor is fine
    // (default move ctor, assignment are fine but not meant to be used)

    void prepend (const T & value) {
        prev->next = rtp::make<list_node>(value, move(prev->next), prev);
        // two things are happening on the line above:
        // 1. ownership of *this is transferred to a new list_node,
        //    prev->next becomes a zero pointer.
        // 2. prev takes ownership of the newly created list_node,
        //    prev->next points to it.
        prev = prev->next;  // final linking of neighbours that the constructor
                            // couldn't take care of (see above).
    }
    void remove_next ( ) {
        auto trash = move(next);   // trash will expire and deallocate *next
        next = move(trash->next);  // we don't want to trash the rest
        if (next != nullptr) next->prev = trash->prev;  // preserve linking
    }
    void remove ( ) { prev->remove_next(); }

    friend list_iterator<T>;
    friend list<T>;

};

template <class T>
class list_iterator {

private:
    using node     = list_node<T>;
    using weak_ptr = typename node::weak_ptr;

    weak_ptr position;  // naturally it doesn't own what it points to

public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = ptrdiff_t;
    using reference       = T&;
    using pointer         = T*;

    list_iterator (weak_ptr p) : position(p) { }
    // default copy ctor, dtor, assignment are all fine

    T & operator*  ( ) { return   position->data;  }
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

};

template <class T>
class list {

private:
    using node     = list_node<T>;
    using data_ptr = typename node::data_ptr;
    using weak_ptr = typename node::weak_ptr;

    data_ptr first = nullptr;  // owns the first node...
    weak_ptr last  = first;    // ... but not the last

public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = ptrdiff_t;
    using reference       = T&;
    using pointer         = T*;
    using iterator        = list_iterator<T>;

    iterator begin ( ) { return rtp::weak(first); }
    iterator end   ( ) { return (last != nullptr) ? last->next : last; }
    // using a nullptr as the end iterator is not a great idea for
    // implementing bidirectional iterators, but it works for the purpose
    // of this demonstration.

    T & front ( ) { return first->data; }
    T & back  ( ) { return last ->data; }

    void push_front (const T & value) {
        if (first != nullptr) {
            first = rtp::make<node>(value, move(first), first->prev);
            first->next->prev = first;
            // note the similarity with list_node::prepend
        }
        else inaugurate(value);
    }
    void push_back (const T & value) {
        if (last != nullptr) {
            last->next = rtp::make<node>(value, move(last->next), last);
            last = last->next;  // no "next" neighbour, relink *this instead
        }
        else inaugurate(value);
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
            // note the similarity with list_node::remove_next
        }
    }
    void pop_back ( ) {
        if      (last == first) pop_front();
        else if (last != nullptr) {
            auto trash = move(last->prev->next);
            // the last node doesn't own anything, so no need for a second
            // ownership transfer
            last = trash->prev;  // relink
        }
    }
    void erase (iterator pos) {
        if      (pos.position == first)     pop_front();
        else if (pos.position != nullptr)   pos.position->remove();
    }

    bool empty ( ) { return first == nullptr; }
    void clear ( ) {
        auto trash = move(first);  // this time we do want to trash everything,
                                   // so no second ownership transfer
        last = first;              // a nullptr because of the move
        // that's all folks!
    }

private:
    void inaugurate (const T & value) {
        first = rtp::make<node>(value, move(first), first);
        last = first;
    }

};

int main ( ) {
    list<int> test;         // { }
    test.push_back(2);      // {2}
    test.push_back(3);      // {2, 3}
    test.push_front(4);     // {4, 2, 3}

    auto pos2 = find(begin(test), end(test), 2);
    test.insert(pos2, 5);   // {4, 5, 2, 3}

    copy(begin(test), end(test), ostream_iterator<int>(cout, " "));  // 4 5 2 3
    cout << endl << test.front() << ' ' << test.back() << endl;      // 4 3

    test.pop_front();       // {5, 2, 3}
    test.erase(pos2);       // {5, 3}
    test.pop_back();        // {5}
    test.push_back(6);      // {5, 6}
    test.clear();           // { }

    cout << test.empty() << endl;  // true

    test.push_back(1);
    test.push_back(2);

    return 0;  // no memory leaks
}
