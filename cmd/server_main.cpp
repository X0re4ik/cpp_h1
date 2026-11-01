#include "h1/io_server/io_server.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        h1::io_server::IOServer server("0.0.0.0", "8080");
        server.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

    return 0;
}