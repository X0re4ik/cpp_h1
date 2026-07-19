#include "h1.hpp"
#include "task.hpp"

#include <getopt.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace
{
struct OptionDesc
{
    option opt;
    const char* help;
};

void printHelp(const OptionDesc* const optionDesc, int optionDescSize)
{
    // NOLINTNEXTLINE (modernize-avoid-c-arrays)
    const char* examples[] = {
        "h1 --add       3 5", "h1 --sub       3 5", "h1 --mul       3 5",
        "h1 --div       3 5", "h1 --factorial 3",   "h1 --power     3 5",
    };

    for (const auto* example : examples) // <- range-based for!
    {
        // NOLINTNEXTLINE (cppcoreguidelines-pro-type-vararg)
        printf("%s\n", example);
    }

    // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
    printf("---\n");

    // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
    for (int i = 0; i != optionDescSize; ++i)
    {
        // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto help = optionDesc[i].help;
        // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto fullCommand = optionDesc[i].opt.name;
        // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto shortCommand = static_cast<char>(optionDesc[i].opt.val);
        if (help != nullptr)
        {
            // NOLINTNEXTLINE (cppcoreguidelines-pro-bounds-pointer-arithmetic)
            printf("--%s (-%c) - %s\n", fullCommand, shortCommand, help);
        }
    }
}
// NOLINTNEXTLINE(readability-non-const-parameter)
void makeTask(int argc, char** argv, Task* task)
{
    task->status = OperationStatus::NOT_STATE;

    const char* const shortOpts = "asmdfph";
    // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    const option longOpts[] = {{"add", no_argument, nullptr, 'a'},
                               {"sub", no_argument, nullptr, 's'},
                               {"mul", no_argument, nullptr, 'm'},
                               {"div", no_argument, nullptr, 'd'},
                               {"factorial", no_argument, nullptr, 'f'},
                               {"power", no_argument, nullptr, 'p'},
                               {"help", no_argument, nullptr, 'h'},
                               {nullptr, 0, nullptr, 0}};
    // NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    const OptionDesc longOptsWithDesc[] = {
        {longOpts[0], "Сложение двух чисел. Example: h1 --add 2 5"},
        {longOpts[1], "Вычитание двух чисел. Example: h1 --sub 2 5"},
        {longOpts[2], "Умножение двух чисел. Example: h1 --mul 2 5"},
        {longOpts[3], "Деление двух чисел. Example: h1 --div 2 5"},
        {longOpts[4], "Вычисление факториала. Example: h1 --factorial 2 5"},
        {longOpts[5], "Врзведение в степень. Example: h1 --power 2 5"},
        {longOpts[6], "Показать эту справку."},
        {longOpts[7], nullptr}};

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    const auto opt = getopt_long(argc, argv, shortOpts,
                                 static_cast<const option*>(longOpts), nullptr);

    if (opt == -1)
    {
        return;
    }

    auto operation = OperationEnum::ADDITION;

    switch (opt)
    {
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case 'a':
            operation = OperationEnum::ADDITION;
            break;
        case 's':
            operation = OperationEnum::SUBTRACTION;
            break;
        case 'm':
            operation = OperationEnum::MULTIPLICATION;
            break;
        case 'd':
            operation = OperationEnum::DIVISION;
            break;
        case 'f':
            operation = OperationEnum::FACTORIAL;
            break;
        case 'p':
            operation = OperationEnum::POWER;
            break;
        case '?':
            // NOLINTNEXTLINE (modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
            printHelp(longOptsWithDesc,
                      sizeof(longOptsWithDesc) / sizeof(longOptsWithDesc[0]));
            setError(task, "Неизвестный флаг");
            return;
        case 'h':
        default:
            // NOLINTNEXTLINE (modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
            printHelp(longOptsWithDesc,
                      sizeof(longOptsWithDesc) / sizeof(longOptsWithDesc[0]));
            return;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto leftValue = static_cast<MathDefault_t>(-1);
    bool isSetLeftValue = false;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto rightValue = static_cast<MathDefault_t>(-1);
    bool isSetRightValue = false;
    // Заполнение первого и второго аргументов
    if (optind < argc)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        leftValue = static_cast<MathDefault_t>(atoi(argv[optind]));
        isSetLeftValue = true;
    }
    if (optind + 1 < argc)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        rightValue = static_cast<MathDefault_t>(atoi(argv[optind + 1]));
        isSetRightValue = true;
    }

    if (!(isSetLeftValue) && !(isSetRightValue))
    {
        setError(task, "Не заданы входные переменные");
        return;
    }

    if (!(isSetRightValue) && (operation != OperationEnum::FACTORIAL))
    {
        setError(task, "Не задан второй параметр");
        return;
    }

    task->left = leftValue;
    task->right = rightValue;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    task->result = static_cast<MathDefault_t>(-1);
    task->operation = operation;
    task->status = OperationStatus::READY;
}

void makeCalculate(Task* task)
{
    if (task->status != OperationStatus::READY)
    {
        return;
    }

    switch (task->operation)
    {
        case OperationEnum::ADDITION:
        {
            h1::makeAddition(task);
            break;
        }
        case OperationEnum::SUBTRACTION:
        {
            h1::makeSubtraction(task);
            break;
        }
        case OperationEnum::MULTIPLICATION:
        {
            h1::makeMultiplication(task);
            break;
        }
        case OperationEnum::DIVISION:
        {
            h1::makeDivision(task);
            break;
        }
        case OperationEnum::FACTORIAL:
        {
            h1::makeFactorial(task);
            break;
        }
        case OperationEnum::POWER:
        {
            h1::makePower(task);
            break;
        }
        default:
        {
            break;
        }
    }
}

void printResult(const Task* const task)
{
    if (task->status == OperationStatus::NOT_STATE)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Запуск без аргументов\n");
        return;
    }
    if (task->status == OperationStatus::OK)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Result: %.2f\n", task->result);
        return;
    }

    if (static_cast<int>(task->status) >=
        static_cast<int>(OperationStatus::ERROR))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Code: %s\n", operationStatus2Str(task->status));
        // NOLINTNEXTLINE (cppcoreguidelines-pro-type-vararg, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        printf("Error: %s\n", task->errorMessage);
        return;
    }
}

void applicationRun(int argc, char** argv)
{
    struct Task* task = createTask();
    // Основная программа
    makeTask(argc, argv, task);
    makeCalculate(task);
    printResult(task);
    // Удаление данных
    deleteTask(task);
}

} // namespace

int main(int argc, char** argv)
{
    applicationRun(argc, argv);
    return 0;
}
