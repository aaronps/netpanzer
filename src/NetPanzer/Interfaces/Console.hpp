#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include <iostream>

class Console
{
public:
    static void initialize();
    static void shutdown();
   
    static std::ostream* server;
};

#endif

