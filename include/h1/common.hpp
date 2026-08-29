#ifndef H_COMMON_75e9988f57d50bef0a3aaf7ebe3eb8f4
#define H_COMMON_75e9988f57d50bef0a3aaf7ebe3eb8f4

#define H1_NODISCARD [[nodiscard]]
#define H1_NOEXCEPT noexcept
#define H1_EXCEPT
#define H1_DEFAULT_WHAT_METHOD const char* what() const noexcept override

#include <exception>
#include <string>

namespace h1
{
using String = std::string;

/**
 * @brief Базовый класс ошибки проекта 
 * 
 */
class H1BaseException : public std::exception
{
  private:
    String msg_;

  public:
    explicit H1BaseException(String msg) : msg_(std::move(msg))
    {}

    H1_DEFAULT_WHAT_METHOD
    {
        return msg_.c_str();
    }
};

} // namespace h1
#endif