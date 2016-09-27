# C++ IRC

This is an unfinished IRC client library in C++. It requires C++14 and depends on SSL and Boost (also uses system-specific threading libraries, so pthreads on Linux).

Currently only supports connecting to SSL-enabled IRC servers.

Usage:

    bfg9000 configure build
    cd build
    ninja
    ./irc <servername> <port>
