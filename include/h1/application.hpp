#ifndef H_H1_APPLICATION_f5eea57a1b697c744fe69c6cd94de14c
#define H_H1_APPLICATION_f5eea57a1b697c744fe69c6cd94de14c

#include "h1/argparse/argparse.hpp"
#include "h1/calculator/calculator.hpp"
#include "h1/json_parser/impl.hpp"
#include "h1/json_parser/json_parser.hpp"

#include <ostream>
#include <string>

namespace h1
{

class Application
{
  public:
    explicit Application(std::ostream& oStream = std::cout,
                         std::ostream& eStream = std::cerr);
    int run(int argc, char** argv);

  private:
    std::reference_wrapper<std::ostream> oStream_;
    std::reference_wrapper<std::ostream> eStream_;
    h1::argp::H1ArgParse argParse_;
};
} // namespace h1

#endif
