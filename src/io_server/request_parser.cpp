#include "h1/io_server/request_parser.hpp"

namespace h1::io_server
{

RequestParser::RequestParser() : state_(State::MethodStart)
{}

void RequestParser::reset()
{
    state_ = State::MethodStart;
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
            return input == '\n';
        default:
            return false;
    }
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
