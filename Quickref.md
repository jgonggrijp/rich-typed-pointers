# Rich-Typed Pointers Quick Reference #


## Inclusion and namespace ##

All types and functions discussed here are in the header `rich_typed_ptr.hpp`, under the namespace `rich_typed_ptr`.


## Pointer types ##

Pointers that are immediately initialized with the return value from a function are best declared `auto`.

`weak_ptr<T>` acts in most ways like `T*`, except that it can only copy the value from another rich-typed pointer and that it cannot be `delete`d.

`owner_ptr<T>` is similar to `weak_ptr<T>` but takes unique ownership of the referenced object. It can only be move-constructed from another `owner_ptr<T>`, usually the return value of a factory function (see below). Assignment is not possible. When it goes out of scope it automatically deallocates the associated object.

`data_ptr<T>` is like `owner_ptr<T>`, but can be explicitly initialized with `nullptr` and can be move-assigned. It should only be used for the implementation of datastructures.


## Factory functions ##

`make<T>(args...)` allocates an object of type `T` and constructs the object with `args...`, then returns a pointer to the object.

`make_dynamic<Base, Derived>(args...)` allocates an object of type `Derived` and constructs it with `args...`, then returns a pointer to `Base` to enable dynamic binding. Requires that `Base` has a virtual destructor.


## Type utilities ##

`weak(ptr)` is a convenience cast function that takes any rich-typed pointer `ptr` to `T` by const reference and returns a `weak_ptr<T>` to the same object, where `T` can be any referencable type.

`ptr_traits<Ptr>` is a template metafunction that provides the following member types if `Ptr` is a rich-typed pointer type:

* `value_type` is an alias of the object type that `Ptr` references, e.g. `int` for `owner_ptr<int>` and `Base` for `owner_ptr<Base>`.
* `weak_ptr_t` is an alias of `weak_ptr<value_type>`.
