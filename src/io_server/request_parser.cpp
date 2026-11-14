#include "h1/io_server/request_parser.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>

namespace h1::io_server
{

namespace
{

/// Сравнение строк без учёта регистра (имена заголовков в HTTP)
bool iequals(std::string_view lhs, std::string_view rhs)
{
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin(), [](char a, char b) {
               return std::tolower(static_cast<unsigned char>(a)) ==
                      std::tolower(static_cast<unsigned char>(b));
           });
}

} // namespace

RequestParser::RequestParser() : state_(State::MethodStart), bodyRemaining_(0)
{}

void RequestParser::reset()
{
    state_ = State::MethodStart;
    bodyRemaining_ = 0;
}

boost::tribool RequestParser::consume(Request& req, char input)
{
    switch (state_)
    {
        case State::MethodStart:
            if (!isChar(input) || isCtl(input) || isTspecial(input))
            {
                return false;
            }
            state_ = State::Method;
            req.method.push_back(input);
            return boost::indeterminate;
        case State::Method:
            if (input == ' ')
            {
                state_ = State::Uri;
                return boost::indeterminate;
            }
            if (!isChar(input) || isCtl(input) || isTspecial(input))
            {
                return false;
            }
            req.method.push_back(input);
            return boost::indeterminate;
        case State::Uri:
            if (input == ' ')
            {
                state_ = State::HttpVersionH;
                return boost::indeterminate;
            }
            if (isCtl(input))
            {
                return false;
            }
            req.uri.push_back(input);
            return boost::indeterminate;
        case State::HttpVersionH:
            if (input == 'H')
            {
                state_ = State::HttpVersionT1;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionT1:
            if (input == 'T')
            {
                state_ = State::HttpVersionT2;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionT2:
            if (input == 'T')
            {
                state_ = State::HttpVersionP;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionP:
            if (input == 'P')
            {
                state_ = State::HttpVersionSlash;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionSlash:
            if (input == '/')
            {
                req.httpVersionMajor = 0;
                req.httpVersionMinor = 0;
                state_ = State::HttpVersionMajorStart;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionMajorStart:
            if (isDigit(input))
            {
                req.httpVersionMajor = req.httpVersionMajor * 10 + input - '0';
                state_ = State::HttpVersionMajor;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionMajor:
            if (input == '.')
            {
                state_ = State::HttpVersionMinorStart;
                return boost::indeterminate;
            }
            if (isDigit(input))
            {
                req.httpVersionMajor = req.httpVersionMajor * 10 + input - '0';
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionMinorStart:
            if (isDigit(input))
            {
                req.httpVersionMinor = req.httpVersionMinor * 10 + input - '0';
                state_ = State::HttpVersionMinor;
                return boost::indeterminate;
            }
            return false;
        case State::HttpVersionMinor:
            if (input == '\r')
            {
                state_ = State::ExpectingNewline1;
                return boost::indeterminate;
            }
            if (isDigit(input))
            {
                req.httpVersionMinor = req.httpVersionMinor * 10 + input - '0';
                return boost::indeterminate;
            }
            return false;
        case State::ExpectingNewline1:
            if (input == '\n')
            {
                state_ = State::HeaderLineStart;
                return boost::indeterminate;
            }
            return false;
        case State::HeaderLineStart:
            if (input == '\r')
            {
                state_ = State::ExpectingNewline3;
                return boost::indeterminate;
            }
            if (!req.headers.empty() && (input == ' ' || input == '\t'))
            {
                state_ = State::HeaderLws;
                return boost::indeterminate;
            }
            if (!isChar(input) || isCtl(input) || isTspecial(input))
            {
                return false;
            }
            req.headers.push_back(Header());
            req.headers.back().name.push_back(input);
            state_ = State::HeaderName;
            return boost::indeterminate;
        case State::HeaderLws:
            if (input == '\r')
            {
                state_ = State::ExpectingNewline2;
                return boost::indeterminate;
            }
            if (input == ' ' || input == '\t')
            {
                return boost::indeterminate;
            }
            if (isCtl(input))
            {
                return false;
            }
            state_ = State::HeaderValue;
            req.headers.back().value.push_back(input);
            return boost::indeterminate;
        case State::HeaderName:
            if (input == ':')
            {
                state_ = State::SpaceBeforeHeaderValue;
                return boost::indeterminate;
            }
            if (!isChar(input) || isCtl(input) || isTspecial(input))
            {
                return false;
            }
            req.headers.back().name.push_back(input);
            return boost::indeterminate;
        case State::SpaceBeforeHeaderValue:
            if (input == ' ')
            {
                state_ = State::HeaderValue;
                return boost::indeterminate;
            }
            return false;
        case State::HeaderValue:
            if (input == '\r')
            {
                state_ = State::ExpectingNewline2;
                return boost::indeterminate;
            }
            if (isCtl(input))
            {
                return false;
            }
            req.headers.back().value.push_back(input);
            return boost::indeterminate;
        case State::ExpectingNewline2:
            if (input == '\n')
            {
                state_ = State::HeaderLineStart;
                return boost::indeterminate;
            }
            return false;
        case State::ExpectingNewline3:
            if (input != '\n')
            {
                return false;
            }
            // Заголовки закончились — определить длину тела
            if (!extractContentLength(req, bodyRemaining_))
            {
                return false;
            }
            if (bodyRemaining_ == 0)
            {
                return true;
            }
            state_ = State::Body;
            return boost::indeterminate;
        case State::Body:
            req.body.push_back(input);
            if (--bodyRemaining_ == 0)
            {
                return true;
            }
            return boost::indeterminate;
        default:
            return false;
    }
}

bool RequestParser::extractContentLength(const Request& req, std::size_t& length)
{
    for (const Header& header : req.headers)
    {
        if (!iequals(header.name, "content-length"))
        {
            continue;
        }
        if (header.value.empty())
        {
            return false;
        }
        std::size_t value = 0;
        for (char c : header.value)
        {
            if (!isDigit(c))
            {
                return false;
            }
            value = value * 10 + static_cast<std::size_t>(c - '0');
        }
        length = value;
        return true;
    }
    // Заголовка нет — тела нет
    length = 0;
    return true;
}

bool RequestParser::isChar(int c)
{
    return c >= 0 && c <= 127;
}

bool RequestParser::isCtl(int c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

bool RequestParser::isTspecial(int c)
{
    switch (c)
    {
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '"':
        case '/':
        case '[':
        case ']':
        case '?':
        case '=':
        case '{':
        case '}':
        case ' ':
        case '\t':
            return true;
        default:
            return false;
    }
}

bool RequestParser::isDigit(int c)
{
    return c >= '0' && c <= '9';
}

} // namespace h1::io_server
