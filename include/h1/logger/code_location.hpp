#ifndef H_H1_LOGGER_CODE_LOCATION_cf398b047c4ad87191d20047e689c446
#define H_H1_LOGGER_CODE_LOCATION_cf398b047c4ad87191d20047e689c446

#include "./enums.hpp"
#include "./exceptions.hpp"
#include "h1/common.hpp"

#include <memory>
#include <optional>
#include <source_location>
#include <string>

namespace h1::log
{

class CodeLocation
{

  public:
    CodeLocation() : lineIn_(-1)
    {}

    // NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
    CodeLocation(int lineIn, const char* filenameIn, const char* functionIn) :
        lineIn_(lineIn), filenameIn_(filenameIn), functionIn_(functionIn)
    {}

    H1_NODISCARD const std::string& getFilenameIn() const H1_NOEXCEPT
    {
        return filenameIn_;
    }

    H1_NODISCARD const std::string& getFunctionIn() const H1_NOEXCEPT
    {
        return functionIn_;
    }

    H1_NODISCARD int getlineIn() const H1_NOEXCEPT
    {
        return lineIn_;
    }

    H1_NODISCARD bool isValid() const H1_NOEXCEPT
    {
        return lineIn_ > 0;
    }

  private:
    int lineIn_;
    std::string filenameIn_;
    std::string functionIn_;
};

} // namespace h1::log

#endif