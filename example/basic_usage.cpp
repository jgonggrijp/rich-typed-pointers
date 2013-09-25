#include <iostream>
using namespace std;

#include "rich_typed_ptr.hpp"
namespace rtp = rich_typed_ptr;
using rtp::weak;

// functions take weak_ptr as arguments, by value is fine
void mutatesomeintptr (rtp::weak_ptr<int> thepointer) {
    *thepointer += 5;
}

// weak_ptr that were taken as argument can of course be returned
rtp::weak_ptr<int> ptr_min (rtp::weak_ptr<int> left, rtp::weak_ptr<int> right) {
    if (*right < *left) return right;
    return left;
}

// generic version of the above
template <class Ptr>
Ptr ptr_min_generic (Ptr left, Ptr right) {
    if (*right < *left) return right;
    return left;
}

// functions return newly created pointers as owner_ptr
rtp::owner_ptr<int> givemeapointer (int value) {
    // rtp::make is the only way to create a rich-typed pointer
    return rtp::make<int>(value);
}

int main ( ) {
    // you can declare all local rich-typed pointers using auto
    auto test1 = rtp::make<int>(4);
    auto test2 = givemeapointer(4);

    mutatesomeintptr(test1);  // test1 automatically casted to weak_ptr
    cout << *test1 << ' ' << *test2 << endl;  // 9 4

    auto test3 = ptr_min(test1, test2);
    cout << *test3 << endl;  // 4

    // due to a limitation in the C++ type system the following won't compile:
    // auto test4 = ptr_min_generic(test1, test2);
    // this can be solved with the convenience cast function `weak`:
    auto test4 = ptr_min_generic(weak(test1), weak(test2));
    cout << *test4 << endl;  // 4

    return 0;  // memory is automatically reclaimed
}
