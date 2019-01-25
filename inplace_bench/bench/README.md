# Abstract

This document aims to describe how the `bench` library works. Every methods are
documented and the ones in `Bench.h` include an exemple.
A code sample of every feature in action is given into [this gist](https://gist.github.com/Dot-H/8b875d4c477f36ad136a23057d4ca1e5).

# `bench` namespace

This namespace brings a group of tool to gather the time elapsed in a task for
each iteration and to store it with associated comments in a human-readable way.
It aims to be precise.

The main advantage of this is that you can put it in a production code and have
no overrhead if the macro ALGOLIA_PROFILING is undefined/set to 0.

The whole thing is implemented around three things:
* The bench objects (Bench.h)
* The bench local tree (BenchTree.h)
* The bench environment (BenchEnv.h)

Informations are stored through a POD class: `BenchInfos` in BenchInfos.h.

# Bench computing classes

The computing classes are:
* BenchTask
* BenchScope
* BenchBunch

Thanks to those classes, a user can have the time taken in a code part and zoom
into a specific part of the function without the noise from the benches.

When creating a bench object or adding a comment, keep in mind to **NEVER** make
function calls or conversions in the arguments (like `std::to_string(smth)`). Due to the evaluation sequence, when the feature is disabled, the compiler would
not be able to optimise out the functions calls and/or the created temporary objects
in the arguments.

Because of this, the name given to a bench must be a literal string (i.e. `const char (&)[N]`)
and the comments must be emplaced post constructor with `Bench*::addComment`.
Note that even if `BenchScope::addComment` is not computed in the elapsed time and
its added overhead is made as small as possible, there is still few nanoseconds of
overhead by call so try to prefer the `BenchTask` class when a lot of comments are 
needed.

# Benchenv

The environment (`BenchEnv`) is a singleton which manages both the global output
buffer and the accesses to local trees. It stores a mapping between the thread
and its local environment.

The environment is also the one managing the buffer containing the JSON
representation of all joined threads.

Every operations are done through the environment. It avoids the user to think
about the threads and which tree he/her should use.

In order to relax the memory usage, the environment makes possible to flush
the JSON buffer.

Note that because the user manages the way the environment's output buffer
is flushed, THE ENVIRONMENT DOES NOT FLUSH ITSELF ON DESTRUCTION. In order to
get the bench, you need to call `BenchEnv::flushBench` at least at the end of
the process.

# Benchtree

Bench objects are stored in a tree local to the thread (`BenchLocalEnv::benchTree`).
The tree stores the bench objects in a way to fit the execution process of the code.
Therefore, a node can only access its parents and the insertion is always in a 
"stacking" way.

When a thread dies, it automatically pushes its tree as a JSON format into the
environment.
