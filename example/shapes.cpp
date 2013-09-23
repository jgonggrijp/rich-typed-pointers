#include <rich_typed_ptr.hpp>
namespace rtp = rich_typed_ptr;

#include <iostream>
#include <string>
using namespace std;

struct shape {
    virtual string describe ( ) = 0;
    virtual ~shape ( ) { }
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
                "are connected by orthogonal angles";
    }
};

int main ( ) {
    auto test1 = rtp::make_dynamic<shape, circle>();
    auto test2 = rtp::make_dynamic<shape, square>();
    cout << test1->describe() << endl << test2->describe() << endl;
    return 0;
}
