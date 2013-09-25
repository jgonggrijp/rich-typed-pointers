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
