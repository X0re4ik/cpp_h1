
#include "h1/application.hpp"
#include "h1/common.hpp"
#include "h1/logger/logger.hpp"

int main(int argc, char* argv[])
{
    auto app = h1::Application();
    auto code = app.run(argc, argv);
    return code;
}
