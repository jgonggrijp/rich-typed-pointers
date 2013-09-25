# Rich-Typed Pointers #

_A smart pointer library that relies on the type system._


## Rationale ##

The purpose of smart pointers is to offer the same flexible access to heap storage as traditional (raw) pointers, while leveraging modern C++ idioms to take away the pitfalls associated with manual resource management. In particular, smart pointers remove the need to explicitly deallocate the associated object when it isn't needed anymore.

As of C++11, the standard library offers two different smart pointer implementations, each with its own virtues. `shared_ptr` allows the same object to be accessed from multiple places and employs reference counting to ensure that allocated objects stay live as long as they are needed. However, it does introduce its own pitfall where circular references may still cause memory leaks, and the reference counting adds overhead at runtime. `unique_ptr` solves these problems by restricting ownership to a single smart pointer instance. However, since it can't be copied it does not allow access from multiple places, unless you break the safety by retrieving the underlying raw pointer through the `get` function.

One may wonder why we can't just have a smart pointer that does everything right. Why can't we have a pointer that manages ownership and deallocation, allows access from multiple places, and does all of that without adding runtime overhead? Indeed, there's nothing that stops us from having such a pointer! All we need to do is rely on the type system.


## How it works ##

The system revolves around two interrelated types, `owner_ptr` and `weak_ptr`.

`owner_ptr` is a bit like `std::unique_ptr` as it takes unique ownership of the object on the heap and it cannot be copied. However, it is even stricter: it can only be move-constructed from another `owner_ptr`. It cannot be default-constructed, it cannot be constructed from a raw pointer (including `nullptr`) and it cannot be assigned. The only way to attach a newly allocated object to a newly constructed `owner_ptr` is through the auxiliary factory function `make`. It doesn't provide any backdoor to access the underlying raw pointer, and under normal use it is guaranteed to reference a live object for its entire lifetime.

> "Normal usage" means that the user doesn't explicitly `move` `owner_ptr`s around.

`weak_ptr` serves a role similar to what `std::weak_ptr` does for `std::shared_ptr`: it provides access without taking ownership. It can be constructed and assigned either from an lvalue `owner_ptr` or from another `weak_ptr` (and nothing else) and can be freely copied. Under normal use it cannot outlive the referenced object. Like `owner_ptr`, it doesn't provide any backdoor; it can safely be assumed to reference a live object.

Within function scope, users normally need not worry about the distinction between these types; they can use `auto` type deduction for all pointers received from other functions and dereference or compare pointers regardless of their type. Functions generally take `weak_ptr`s as parameters. `owner_ptr` automatically casts to `weak_ptr` when passed to such a parameter. Functions return `weak_ptr` if the pointer was previously taken as an argument, or `owner_ptr` if it was allocated within the function body.

This sums up the basic mechanics of rich-typed pointers. For an illustration, see `example/basic_usage.cpp`.


### Datastructures ###

Those who paid close attention while reading the description above may think that `owner_ptr` is too restrictive for the implementation of datastructures. Indeed, datastructures generally require that pointers can be zero-initialized and `owner_ptr` does not allow this. Fortunately there is a solution.

For the specific purpose of implementing datastructures `owner_ptr` has a slightly more permissive sibling, `data_ptr`. `data_ptr` is almost identical to `owner_ptr`, including the ability to initialize from `make` (or `make_dynamic`, see below) and to be cast to `weak_ptr`. In addition it can be explicitly initialized from `nullptr` and allows move assignment.

> Note that the added freedom that `data_ptr` provides should really only be needed for the implementation of datastructures. Using it for any other purpose indicates a design fault.

A fairly elaborate illustration of the usage of `data_ptr` is provided in `example/linked_list.cpp`.


### Dynamic binding ###

For the purpose of object-oriented programming the rich-typed pointer library implements an alternative factory function, `make_dynamic`. Given two types `base` and `derived`, this function will create a pointer to `base` that references an object on the heap of type `derived`. `make_dynamic` ensures that `derived` is truly a subclass of `base` and that `base` has a virtual destructor. For an illustration, see `example/shapes.cpp`.

As of yet there is no way to downcast a rich-typed pointer created with `make_dynamic`. However, such a feature can be added if there is sufficient demand.


### Arrays ###

All templates in `rich_typed_ptr.hpp` will be specialized for array types. This is not implemented yet.


### Custom allocators ###

The rich-typed pointer library will support custom allocators. This is not implemented yet.


### Storage in containers ###

`owner_ptr`, `data_ptr` and `weak_ptr` can all be safely stored in containers. In the former two cases the container owns all objects referenced by the pointers it stores, in the latter case it does not.


### Cyclic references ###

Under normal usage, cyclic ownership is impossible so it cannot lead to memory leaks. Cyclic referencing through `weak_ptr`s is possible but unproblematic.

Cyclic ownership using `data_ptr` and `move` is possible with some effort, but cannot happen by accident. The datastructure designer is forced to consider what owns what, and naieve mistakes that would lead to cyclic ownership when using `std::shared_ptr` will not compile when using rich typed pointers. When cyclic ownership is created by design, it can also be undone. Cyclic ownership by design is illustrated in `example/linked_ring.cpp`.

Note that rich typed pointers can never be part of multiple ownership cycles at the same time. This is a true impossibility because at any time exactly one pointer will own a given object.


### Thread safety ###

Dangling pointers and null pointers are guaranteed not to occur under normal usage if one of the following strategies is adopted:

1. A pointer is owned by the common ancestor of all threads that share access to it, at or above the scope where all sharing threads are launched and joined.
2. Threads transfer ownership in order to access a pointer, and non-owning threads do not keep any `weak_ptr` to the referenced object.

As with all smart pointers, it is the responsibility of the user to prevent race conditions on the referenced object. The inherent synchronization issue associated with reference counting does not apply to rich typed pointers.


### Exception safety ###

`make` and `make_dynamic` will usually offer some degree of exceptions safety, depending on the allocator and the constructor of the object being allocated. Move and copy operations, the comparison operators and `weak` offer the no-throw guarantee. All other operations offer the basic no-leak guarantee on the condition that the referenced object has a non-throwing destructor. None of the functions in the library will throw exceptions out of themselves.


## How it could be even better ##

The C++ type system has two shortcomings that prevent rich-typed pointers from being the ultimate safe solution to nearly all use cases. Solving the first shortcoming would remove the need for explicit typecasts to `weak_ptr`. Solving the second shortcoming would remove the need for the runtime assertions that are currently included in the code, by catching the corresponding errors at compile time.


### Better type deduction ###

The `auto` keyword can be used when an `owner_ptr` is created:

    auto foo = make<int>();

The compiler determines that the type of the expression `make<int>()` is `owner_ptr<int>` and correctly assigns this type to `foo`. However, the following fails to compile:

    auto bar = foo;

The C++ standard dictates that the type of `bar` should be deduced by inferring the type of the right-hand side of the assignment, which is `owner_ptr<int>`. This makes the entire statement a copy assignment of `owner_ptr<int>` to `owner_ptr<int>`, which is not allowed. However, any human reader with knowledge of the rich-typed pointer library can see that the type of `bar` should be `weak_ptr<int>`. What the `auto` keyword really should be doing is to take the entire statement into account, by looking for a type that can substitute the question mark in the following constructor prototype:

    ? (const owner_ptr<int> &);

and then it would unambiguously find `weak_ptr<int>`. Since this is not what `auto` does, we have to write this workaround:

    auto bar = weak(foo);

The same problem occurs in template parameter type resolution. Consider the following code that does not compile:

    template <class Ptr>
    void baz (Ptr arg) { std::cout << *arg << std::endl; }

    baz(foo);  // foo is still of type owner_ptr<int>

The standard again dictates that `Ptr` should simply be resolved to the type of `foo`, even though the compiler could use the suggestion that `baz` takes it argument by value and that it knows a type which can be constructed from an lvalue `owner_ptr<int>`. Because it doesn't we have to use `weak` again:

    baz(weak(foo));


### Dependent typing ###

> Dependent typing should not be confused with dynamic typing. In dynamic typing the type of an object is enforced at runtime. In dependent typing, the type of an object can depend on its context but it may still be enforced at compile time.

In C++, once an object is declared it cannot change type. Consequently, if we want to allow certain operations on the object during certain parts of its lifetime, we have to anticipate this by giving it a type that will allow those operations for its *entire* lifetime -- even if those operations would be semantically invalid for most of its lifetime. This leaves the possibility of problematic code like the following:

    template <class T> void baz (weak_ptr<T>);

    auto foo = data_ptr<int>(nullptr);
    *foo;             // invalid, but will only be detected at runtime
    baz(weak(foo));   // compiles, baz cannot safely assume that its
                      // argument is not-null

    foo = make<int>(0);
    *foo;                   // still compiles, now valid
    baz(weak(foo));         // compiler detects no difference

    auto bar = move(foo);   // foo becomes null again
    *foo;                   // invalid, still compiles
    auto ooz = weak(foo);   // compiles even though risky
    *ooz;                   // also compiles, but invalid!

If we had dependent types in C++ we could eliminate `data_ptr`, remove all runtime assertions and have this instead:

    template <class T> void baz (weak_ptr<T>);

    auto foo = nullptr;     // nullptr_t
    *foo;                   // does not compile
    baz(weak(foo));         // error, no conversion known

    foo = make<int>(0);     // becomes owner_ptr<int>
    *foo;                   // compiles
    baz(weak(foo));         // fine

    auto bar = move(foo);   // foo becomes nullptr_t again
    *foo;                   // compiler error
    auto ooz = weak(foo);   // error, no conversion known
    auto ooz = foo;         // ooz is also nullptr_t
    *ooz;                   // compiler error

Dependent typing is not trivial to implement, but as long as the possibility for an object to change type is restricted to other types that have the same underlying binary representation, it should be doable. Doing just that would make C++ an even more powerful language than it is today.
