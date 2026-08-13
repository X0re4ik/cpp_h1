#ifndef H_H1_LOGGER_c97039f889419433ec1c11a4e8f5a548
#define H_H1_LOGGER_c97039f889419433ec1c11a4e8f5a548

#include <memory>
#include <string>

#define H1_LOGGER_CODE_LOCATION                                                \
    h1_logger::CodeLocation                                                    \
    {                                                                          \
        static_cast<int>(__LINE__), __FILE__,                                  \
            static_cast<const char*>(__FUNCTION__)                             \
    }

#define H1_LOGGER_INTERNAL_PROJECT_LOG(level, msg)                             \
    do                                                                         \
    {                                                                          \
        h1_logger::ProjectLoggerFactory::getInstance()->level(                 \
            msg, (H1_LOGGER_CODE_LOCATION));                                   \
    } while (0);

#define LOG_INFO(msg) H1_LOGGER_INTERNAL_PROJECT_LOG(info, msg)
#define LOG_DEBUG(msg) H1_LOGGER_INTERNAL_PROJECT_LOG(debug, msg)
#define LOG_WARN(msg) H1_LOGGER_INTERNAL_PROJECT_LOG(warn, msg)
#define LOG_ERROR(msg) H1_LOGGER_INTERNAL_PROJECT_LOG(error, msg)

namespace h1_logger
{

enum LoggerLevelEnum
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    CRITICAL = 5,
    OFF = 6
};

class CodeLocation
{

  public:
    CodeLocation() : lineIn_(-1)
    {}

    // NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
    CodeLocation(int lineIn, const char* filenameIn, const char* functionIn) :
        lineIn_(lineIn), filenameIn_(filenameIn), functionIn_(functionIn)
    {}

    [[nodiscard]] const std::string& getFilenameIn() const
    {
        return filenameIn_;
    }

    [[nodiscard]] const std::string& getFunctionIn() const
    {
        return functionIn_;
    }

    [[nodiscard]] const int& getlineIn() const
    {
        return lineIn_;
    }

  private:
    int lineIn_;
    std::string filenameIn_;
    std::string functionIn_;
};

class ILogSink
{
  public:
    using fString = std::string;

    // Constructors
    ILogSink() = default;

    // Copy constructor/operator=
    ILogSink(const ILogSink&) = delete;
    ILogSink& operator=(const ILogSink&) = delete;

    // Move constructor/operator=
    ILogSink(ILogSink&&) = delete;
    ILogSink& operator=(ILogSink&&) = delete;

    // Destructor
    virtual ~ILogSink() = default;

    /**
     * @brief Логгирование уровня trace
     * 
     * @param fmt Строка формата "Привет, я залогировал 1 2 3" 
     */
    virtual void trace(const fString&, const CodeLocation&) const = 0;

    /**
     * @brief Логгирование уровня debug
     * 
     * @param fmt Строка формата "Привет, я залогировал 1 2 3" 
     */
    virtual void debug(const fString&, const CodeLocation&) const = 0;

    /**
     * @brief Логгирование уровня info
     * 
     * @param fmt Строка формата "Привет, я залогировал 1 2 3" 
     */
    virtual void info(const fString&, const CodeLocation&) const = 0;

    /**
     * @brief Логгирование уровня warn
     * 
     * @param fmt Строка формата "Привет, я залогировал 1 2 3" 
     */
    virtual void warn(const fString&, const CodeLocation&) const = 0;

    /**
     * @brief Логгирование уровня error
     * 
     * @param fmt Строка формата "Привет, я залогировал 1 2 3" 
     */
    virtual void error(const fString&, const CodeLocation&) const = 0;

    /**
     * @brief Set the Level object
     * 
     */
    virtual void setLevel(const LoggerLevelEnum&) const = 0;
};

class ProjectLoggerFactory
{

  public:
    using PtrProjectLogger = std::shared_ptr<ILogSink>;

    /**
     * @brief Get the Instance object
     * 
     * @return const PtrProjectLogger& 
     */
    static const PtrProjectLogger& getInstance();

  private:
    static PtrProjectLogger createInstance();
};

} // namespace h1_logger

#endif