This repository contains the code corresponding to the paper
[Omnisc’IO: A Grammar-Based Approach to Spatial and Temporal I/O Patterns Prediction](https://ieeexplore.ieee.org/document/7013038),
by Matthieu Dorier, Shadi Ibrahim, Gabriel Antoniu, Robert Ross,
IEEE/ACM International Conference for High Performance Computing, Networking, Storage and Analysis (SC14).
I put it online because some students have reached out recently to ask for the code.

Note that this version does not contain the "star-sequitur" optimization, presented in the follow-up TPDS paper
[Using Formal Grammars to Predict I/O Behaviors in HPC: the Omnisc’IO Approach](https://ieeexplore.ieee.org/document/7289462).

If you need help understanding this code, all I can say is "good luck".
It's been 8 years since I've coded this, and back then I spent days in front of
a white board figuring out the details and debugging it, so my current knowledge
of it pretty much gone.

# Omnisc'IO

This version provides the following working features:
- Implementation of the Sequitur algorithm for grammar inference.
- Improvements to the Sequitur algorithm to support prediction of future symbols.
- Access tables to track sizes and offsets of I/O operations, as well as inter-arrival time.

These features are used in src/sequitur/analyzer.cpp
to perform an offline analysis of I/O logs. See also
test/reader.cpp for an example of code building a
grammar and performing predictions.

Wrappers to I/O interfaces (POSIX and MPI-I/O) are under development.

## Compiling

```
cmake -G "Unix Makefiles"
make
```

## Using the "reader" example

```
./reader file
```

Where file contains a stream of characters (these
characters will be converted into integer values in
the program's output).

## License

Omnisc'IO is under the terms and conditions of the
BSD license (see headers in source files).
Parts of Omnisc'IO (identified in files
headers) are inspired by Sequitur, by C. Nevill-
Manning, under the Apache 2.0 license. Sequitur's
original code can be found at
http://www.sequitur.info/sequitur_simple.cc
