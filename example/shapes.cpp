/*
    Copyright 2013 Julian Gonggrijp
    Version 0, alpha 1.
*/

#include <rich_typed_ptr.hpp>
namespace rtp = rich_typed_ptr;

#include <iostream>
#include <string>
using namespace std;

struct shape {
    virtual string describe ( ) = 0;
    virtual ~shape ( ) { }  // necessary, would not compile otherwise
};

struct circle : shape {
    string describe ( ) {
        return  "I'm curvy and all points on my perimeter "
                "are equidistant from my center";
    }
};

struct square : shape {
    string describe ( ) {
        return  "My four sides, all straight and equally long, "
                "are connected by orthogonal corners";
    }
};

int main ( ) {
    // rtp::make_dynamic<base, derived> returns a pointer to base
    // which references an object of type derived
    auto test1 = rtp::make_dynamic<shape, circle>();
    auto test2 = rtp::make_dynamic<shape, square>();
    cout << test1->describe() << endl << test2->describe() << endl;

    return 0;  // memory neatly returned here
}
