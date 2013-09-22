#include <iostream>
using namespace std;

#include "rich_typed_ptr.hpp"
namespace rtp = rich_typed_ptr;

// functions take weak_ptr as arguments
void mutatesomeintptr (rtp::weak_ptr<int> thepointer) {
    *thepointer += 5;
}

// weak_ptr that were taken as argument can of course be returned
template <class Ptr>
Ptr ptr_min (Ptr left, Ptr right) {
    if (*right < *left) return right;
    return left;
}

// functions return newly created pointers as owner_ptr
rtp::owner_ptr<int> givemeapointer (int value) {
    return rtp::make<int>(value);
}

int main ( ) {
    auto test1 = givemeapointer(4);

    // explicit copy from an owner_ptr requires explicit type
    auto test2 = rtp::weak<int>(test1);

    cout << *test1 << ' ' << *test2 << endl;  // 4 4
    mutatesomeintptr(test1);
    cout << *test1 << ' ' << *test2 << endl;  // 9 9
    mutatesomeintptr(test2);
    cout << *test1 << ' ' << *test2 << endl;  // 14 14

    // explicit copy from a weak_ptr allows auto type inference
    auto test3 = test2;

    auto test4 = rtp::make<int>(4);
    auto test5 = rtp::weak<int>(test4);
    auto test6 = ptr_min(test3, test5);
    cout << *test6 << endl;  // 4

    return 0;  // memory is automatically reclaimed
}
