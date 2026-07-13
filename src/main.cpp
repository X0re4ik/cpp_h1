#include "h1.hpp"

#include <getopt.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace
{

bool parseDouble(const char* text, double* out)
{
    if ((text == nullptr) || (out == nullptr))
    {
        return false;
    }

    // NOLINTNEXTLINE(misc-const-correctness)
    char* endPtr = nullptr;
    errno = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    const double parsed = strtod(text, &endPtr);

    if (endPtr == text)
    {
        // Ни один символ не был распознан как число.
        return false;
    }
    if (*endPtr != '\0')
    {
        // После числа остались лишние символы, например "5abc".
        return false;
    }
    if (errno == ERANGE)
    {
        // Число не помещается в double.
        return false;
    }

    *out = parsed;
    return true;
}

//NOLINTNEXTLINE (clang-tidy(bugprone-easily-swappable-parameters)
bool checkValidValues(bool leftParsed, bool rightParsed,
                      OperationEnum operation)
{
    if (!leftParsed)
    {
        return false;
    }
    if (!rightParsed && (operation != OperationEnum::FACTORIAL))
    {
        return false;
    }
    return true;
}

struct OptionDesc
{
    option opt;
    const char* help;
};

void printHelp(const OptionDesc* const optionDesc, int optionDescSize)
{
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

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* operation = new OperationEnum{OperationEnum::ADDITION};

    switch (opt)
    {
        case 'a':
            *operation = OperationEnum::ADDITION;
            break;
        case 's':
            *operation = OperationEnum::SUBTRACTION;
            break;
        case 'm':
            *operation = OperationEnum::MULTIPLICATION;
            break;
        case 'd':
            *operation = OperationEnum::DIVISION;
            break;
        case 'f':
            *operation = OperationEnum::FACTORIAL;
            break;
        case 'p':
            *operation = OperationEnum::POWER;
            break;
        case 'h':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            printHelp(longOptsWithDesc,
                      sizeof(longOptsWithDesc) / sizeof(longOptsWithDesc[0]));
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete operation;
            task->status = OperationStatus::HELP_SHOWN;
            return;
        case '?':
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete operation;
            task->status = OperationStatus::ERROR_PARSE_VALUES;
            return;
        default:
            break;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* leftValue = new double{0.0};
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    auto* rightValue = new double{0.0};

    bool leftParsed = false;
    bool rightParsed = false;

    // Заполнение первого и второго аргументов
    if (optind < argc)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        leftParsed = parseDouble(argv[optind], leftValue);
    }
    if (optind + 1 < argc)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        rightParsed = parseDouble(argv[optind + 1], rightValue);
    }

    if (!checkValidValues(leftParsed, rightParsed, *operation))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete leftValue;
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete rightValue;
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete operation;
        task->status = OperationStatus::ERROR_PARSE_VALUES;
        return;
    }

    task->left = leftValue;
    task->right = rightValue;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    task->result = new double{0};
    task->operation = operation;
    task->status = OperationStatus::READY;
}

void makeCalculate(Task* task)
{
    if (task->status != OperationStatus::READY)
    {
        return;
    }

    switch (*task->operation)
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
            task->status = OperationStatus::ERROR_UNKNOWN_OPERATION;
            break;
        }
    }
}

void printResult(const Task* const task)
{
    if (task->status == OperationStatus::HELP_SHOWN)
    {
        // Текст справки уже выведен внутри printHelp - здесь печатать
        // больше нечего.
        return;
    }
    if (task->status == OperationStatus::NOT_STATE)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Запуск без аргументов\n");
        return;
    }
    if (task->status == OperationStatus::OK)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("The result of the program: %.2f\n", *task->result);
        return;
    }

    if (static_cast<int>(task->status) >=
        static_cast<int>(OperationStatus::ERROR))
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        printf("Program execution error: %s\n",
               operationStatusToString(task->status));
        return;
    }
}

void applicationRun(int argc, char** argv)
{
    auto* task = createTask();
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
