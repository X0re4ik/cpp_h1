#include <cfloat>
#include <cstdint>

enum class OperationEnum : uint8_t
{
    ADDITION = 1,
    SUBTRACTION = 2,
    MULTIPLICATION = 3,
    DIVISION = 4,
    FACTORIAL = 5,
    POWER = 6,
};

enum class OperationStatus : uint16_t
{
    NOT_STATE = 0,
    READY = 100,
    HELP_SHOWN = 150,
    OK = 200,
    ERROR = 500,
    ERROR_PARSE_VALUES = 505,
    ERROR_UNKNOWN_OPERATION = 510,
    ERROR_OVERFLOW = 520
};

using MathDefault_t = double;

inline bool isOverflowResult(MathDefault_t value)
{
    // Если value = NaN, то NaN != NaN
    if (value != value)
    {
        return true;
    }
    return (value > DBL_MAX) || (value < -DBL_MAX);
}

inline const char* operationStatusToString(OperationStatus operation)
{
    switch (operation)
    {
        case OperationStatus::NOT_STATE:
            return "NOT_STATE";
        case OperationStatus::READY:
            return "READY";
        case OperationStatus::HELP_SHOWN:
            return "HELP_SHOWN";
        case OperationStatus::OK:
            return "OK";
        case OperationStatus::ERROR:
            return "ERROR";
        case OperationStatus::ERROR_PARSE_VALUES:
            return "ERROR_PARSE_VALUES";
        case OperationStatus::ERROR_UNKNOWN_OPERATION:
            return "ERROR_UNKNOWN_OPERATION";
        default:
            return "NOT_FOUND";
    }
}

struct Task
{
    MathDefault_t* left;
    MathDefault_t* right;
    MathDefault_t* result;
    OperationEnum* operation;
    OperationStatus status;
};

inline Task* createTask()
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* newTask = new Task{
        nullptr, nullptr, nullptr, nullptr, OperationStatus::NOT_STATE,
    };
    return newTask;
}

inline void deleteTask(Task* task)
{
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete task->left;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete task->right;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete task->result;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete task->operation;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    delete task;
}

namespace h1
{
inline void makeAddition(struct Task* task);
inline void makeSubtraction(struct Task* task);
inline void makeMultiplication(struct Task* task);
inline void makeDivision(struct Task* task);
inline void makeFactorial(struct Task* task);
inline void makePower(struct Task* task);
} // namespace h1

void h1::makeAddition(struct Task* task)
{
    *task->result = *task->left + *task->right;
    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }
    task->status = OperationStatus::OK;
}

void h1::makeSubtraction(struct Task* task)
{
    *task->result = *task->left - *task->right;
    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }
    task->status = OperationStatus::OK;
}

void h1::makeMultiplication(struct Task* task)
{
    *task->result = *task->left * *task->right;
    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }
    task->status = OperationStatus::OK;
}

void h1::makeDivision(struct Task* task)
{
    if (*task->right == 0)
    {
        task->status = OperationStatus::ERROR;
        return;
    }
    *task->result = *task->left / *task->right;
    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }
    task->status = OperationStatus::OK;
}

void h1::makeFactorial(struct Task* task)
{
    if (*task->left < 0)
    {
        task->status = OperationStatus::ERROR;
        return;
    }

    auto value = static_cast<int>(*task->left);
    int result = 1;
    for (int i = 1; i <= value; ++i)
    {
        result *= i;
    }
    *task->result = result;

    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }

    task->status = OperationStatus::OK;
}

void h1::makePower(struct Task* task)
{
    if (*task->right < 0)
    {
        task->status = OperationStatus::ERROR;
        return;
    }
    auto exp = static_cast<int>(*task->right);
    MathDefault_t result = 1.0;
    for (int i = 0; i < exp; ++i)
    {
        result *= *task->left;
    }

    *task->result = result;
    if (isOverflowResult(*task->result))
    {
        task->status = OperationStatus::ERROR_OVERFLOW;
        return;
    }
    task->status = OperationStatus::OK;
}