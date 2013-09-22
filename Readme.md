# Rich-typed pointers #

_A smart pointer library that relies on the type system._


## Rationale ##

The purpose of smart pointers is to offer the same flexible access to heap storage as traditional (raw) pointers, while leveraging modern C++ idioms to take away the pitfalls associated with manual resource management. In particular, smart pointers remove the need to explicitly deallocate the associated object when it isn't needed anymore.

As of C++11, the standard library offers two different smart pointer implementations, each with its own virtues. `shared_ptr` allows the same object to be accessed from multiple places and employs reference counting to ensure that allocated objects stay live as long as they are needed. However, it does introduce its own pitfall where circular references may still cause memory leaks, and the reference counting adds overhead at runtime. `unique_ptr` solves these problems by restricting ownership to a single smart pointer instance. However, since it can't be copied it does not allow access from multiple places.

One may wonder why we can't just have a smart pointer that does everything right. Why can't we have a pointer that manages ownership and deallocation, allows access from multiple places, and does all of that without adding runtime overhead? Indeed, there's nothing that stops us from having such a pointer! All we need to do is rely on the type system.
