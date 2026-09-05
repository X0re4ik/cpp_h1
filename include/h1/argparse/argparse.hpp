#ifndef H_H1_ARGPARSE_09f9c08ea7bb1a76cc403d05a2939e73
#define H_H1_ARGPARSE_09f9c08ea7bb1a76cc403d05a2939e73

#include "h1/common.hpp"

namespace h1::argp
{

struct ArgsValue
{
    String jsonValue;
    bool verbose;
};

class H1ArgParse
{
  public:
    explicit H1ArgParse(String programName);
    H1ArgParse(String programName, String version);

    // NOLINTNEXTLINE(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
    ArgsValue parse(int argc, char* argv[]) H1_EXCEPT;

  private:
    String programName_;
    String version_;
};

} // namespace h1::argp

#endif