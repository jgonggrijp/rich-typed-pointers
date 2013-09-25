#include <rich_typed_ptr.hpp>
namespace rtp = rich_typed_ptr;

#include <iostream>
#include <utility>
using namespace std;

template <class T>
class ring {        // singly linked cyclical ownership

private:
    struct node {   // just a regular owning link node
        using data_ptr = typename rtp::data_ptr<node>;

        T           data;
        data_ptr    next;

        node (const T & init, data_ptr && successor)
            : data(init), next(forward<data_ptr>(successor)) { }
        // implicitly defined destructor is fine
    };

    using data_ptr = typename node::data_ptr;
    using weak_ptr = rtp::weak_ptr<node>;

public:
    ring ( ) : handle(data_ptr(nullptr)) { }
    ring (const T & init) : handle(data_ptr(nullptr)) {
        inaugurate(init);  // creates the cycle
    }
    ~ring ( ) {
        // break the cycle
        if (handle != nullptr) auto trash = move(handle->next);
        // trash expires and takes the ownership chain with it
    }

    // move semantics
    ring (ring && predecessor) : handle(predecessor.handle) {
        predecessor.handle = data_ptr(nullptr);
    }
    ring & operator= (ring && trader) {
        swap(handle, trader.handle);
    }
    // (Copying can be done, this requires creating a non-owning ring
    // type that can be constructed from the owning ring type. Note that
    // this basically amounts to implementing meta-rich typed pointers!)

    // basic iterator emulation
    // note that conceptually, handle->next is the "current" node
    ring & operator++ ( )       { handle = handle->next; return *this; }
    T &    operator*  ( ) const { return   handle->next->data ; }
    T *    operator-> ( ) const { return &(handle->next->data); }

    // adding and removing data
    void push (const T & insertion) {
        if (handle == nullptr) {
            inaugurate(insertion);
        } else {
            handle->next = rtp::make<node>(insertion, move(handle->next));
            ++*this;
        }
    }
    void pop ( ) {
        if (handle == nullptr) return;
        auto trash = move(handle->next);
        handle->next = move(trash->next);
        if (handle->next == nullptr) handle = data_ptr(nullptr);
    }

private:
    weak_ptr handle;

    // create cyclical ownership: this is hard, as it should be!
    void inaugurate (const T & init) {
        auto temp = rtp::make<node>(init, nullptr);
        handle = temp;
        handle->next = move(temp);
    }

};

int main ( ) {
    auto test = ring<int>(1);
    test.push(2);
    test.push(3);
    test.push(4);
    cout << *test << *++test << *++test << *++test << endl;  // 1234

    (++test).pop();
    cout << *test << *++test << *++test << *++test << endl;  // 2342

    return 0;  // cycle broken and memory reclaimed thanks to custom dtor
}
