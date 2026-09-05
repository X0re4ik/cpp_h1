#ifndef H_H1_APPLICATION_EXCEPTIONS_1522299802a642b861f58e28b20b5854
#define H_H1_APPLICATION_EXCEPTIONS_1522299802a642b861f58e28b20b5854

#include "h1/common.hpp"

namespace h1
{

class AppException : public H1BaseException
{
  public:
    explicit AppException(String msg) : H1BaseException(std::move(msg))
    {}
};

} // namespace h1

#endif