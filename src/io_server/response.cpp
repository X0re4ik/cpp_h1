#include "h1/io_server/response.hpp"

#include <boost/lexical_cast.hpp>

namespace h1::io_server
{

namespace statusStrings
{

const String ok = "HTTP/1.0 200 OK\r\n";
const String created = "HTTP/1.0 201 Created\r\n";
const String accepted = "HTTP/1.0 202 Accepted\r\n";
const String noContent = "HTTP/1.0 204 No Content\r\n";
const String multipleChoices = "HTTP/1.0 300 Multiple Choices\r\n";
const String movedPermanently = "HTTP/1.0 301 Moved Permanently\r\n";
const String movedTemporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
const String notModified = "HTTP/1.0 304 Not Modified\r\n";
const String badRequest = "HTTP/1.0 400 Bad Request\r\n";
const String unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
const String forbidden = "HTTP/1.0 403 Forbidden\r\n";
const String notFound = "HTTP/1.0 404 Not Found\r\n";
const String internalServerError = "HTTP/1.0 500 Internal Server Error\r\n";
const String notImplemented = "HTTP/1.0 501 Not Implemented\r\n";
const String badGateway = "HTTP/1.0 502 Bad Gateway\r\n";
const String serviceUnavailable = "HTTP/1.0 503 Service Unavailable\r\n";

boost::asio::const_buffer toBuffer(Response::Status status)
{
    switch (status)
    {
        case Response::Status::Ok:
            return boost::asio::buffer(ok);
        case Response::Status::Created:
            return boost::asio::buffer(created);
        case Response::Status::Accepted:
            return boost::asio::buffer(accepted);
        case Response::Status::NoContent:
            return boost::asio::buffer(noContent);
        case Response::Status::MultipleChoices:
            return boost::asio::buffer(multipleChoices);
        case Response::Status::MovedPermanently:
            return boost::asio::buffer(movedPermanently);
        case Response::Status::MovedTemporarily:
            return boost::asio::buffer(movedTemporarily);
        case Response::Status::NotModified:
            return boost::asio::buffer(notModified);
        case Response::Status::BadRequest:
            return boost::asio::buffer(badRequest);
        case Response::Status::Unauthorized:
            return boost::asio::buffer(unauthorized);
        case Response::Status::Forbidden:
            return boost::asio::buffer(forbidden);
        case Response::Status::NotFound:
            return boost::asio::buffer(notFound);
        case Response::Status::InternalServerError:
            return boost::asio::buffer(internalServerError);
        case Response::Status::NotImplemented:
            return boost::asio::buffer(notImplemented);
        case Response::Status::BadGateway:
            return boost::asio::buffer(badGateway);
        case Response::Status::ServiceUnavailable:
            return boost::asio::buffer(serviceUnavailable);
        default:
            return boost::asio::buffer(internalServerError);
    }
}

} // namespace statusStrings

namespace miscStrings
{

const char nameValueSeparator[] = {':', ' '};
const char crlf[] = {'\r', '\n'};

} // namespace miscStrings

std::vector<boost::asio::const_buffer> Response::toBuffers()
{
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(statusStrings::toBuffer(status));
    for (Header& header : headers)
    {
        buffers.push_back(boost::asio::buffer(header.name));
        buffers.push_back(boost::asio::buffer(miscStrings::nameValueSeparator));
        buffers.push_back(boost::asio::buffer(header.value));
        buffers.push_back(boost::asio::buffer(miscStrings::crlf));
    }
    buffers.push_back(boost::asio::buffer(miscStrings::crlf));
    buffers.push_back(boost::asio::buffer(content));
    return buffers;
}

namespace stockReplies
{

const char ok[] = "";
const char created[] = "<html>"
                       "<head><title>Created</title></head>"
                       "<body><h1>201 Created</h1></body>"
                       "</html>";
const char accepted[] = "<html>"
                        "<head><title>Accepted</title></head>"
                        "<body><h1>202 Accepted</h1></body>"
                        "</html>";
const char noContent[] = "<html>"
                         "<head><title>No Content</title></head>"
                         "<body><h1>204 Content</h1></body>"
                         "</html>";
const char multipleChoices[] = "<html>"
                               "<head><title>Multiple Choices</title></head>"
                               "<body><h1>300 Multiple Choices</h1></body>"
                               "</html>";
const char movedPermanently[] = "<html>"
                                "<head><title>Moved Permanently</title></head>"
                                "<body><h1>301 Moved Permanently</h1></body>"
                                "</html>";
const char movedTemporarily[] = "<html>"
                                "<head><title>Moved Temporarily</title></head>"
                                "<body><h1>302 Moved Temporarily</h1></body>"
                                "</html>";
const char notModified[] = "<html>"
                           "<head><title>Not Modified</title></head>"
                           "<body><h1>304 Not Modified</h1></body>"
                           "</html>";
const char badRequest[] = "<html>"
                          "<head><title>Bad Request</title></head>"
                          "<body><h1>400 Bad Request</h1></body>"
                          "</html>";
const char unauthorized[] = "<html>"
                            "<head><title>Unauthorized</title></head>"
                            "<body><h1>401 Unauthorized</h1></body>"
                            "</html>";
const char forbidden[] = "<html>"
                         "<head><title>Forbidden</title></head>"
                         "<body><h1>403 Forbidden</h1></body>"
                         "</html>";
const char notFound[] = "<html>"
                        "<head><title>Not Found</title></head>"
                        "<body><h1>404 Not Found</h1></body>"
                        "</html>";
const char internalServerError[] =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
const char notImplemented[] = "<html>"
                              "<head><title>Not Implemented</title></head>"
                              "<body><h1>501 Not Implemented</h1></body>"
                              "</html>";
const char badGateway[] = "<html>"
                          "<head><title>Bad Gateway</title></head>"
                          "<body><h1>502 Bad Gateway</h1></body>"
                          "</html>";
const char serviceUnavailable[] =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";

String toString(Response::Status status)
{
    switch (status)
    {
        case Response::Status::Ok:
            return ok;
        case Response::Status::Created:
            return created;
        case Response::Status::Accepted:
            return accepted;
        case Response::Status::NoContent:
            return noContent;
        case Response::Status::MultipleChoices:
            return multipleChoices;
        case Response::Status::MovedPermanently:
            return movedPermanently;
        case Response::Status::MovedTemporarily:
            return movedTemporarily;
        case Response::Status::NotModified:
            return notModified;
        case Response::Status::BadRequest:
            return badRequest;
        case Response::Status::Unauthorized:
            return unauthorized;
        case Response::Status::Forbidden:
            return forbidden;
        case Response::Status::NotFound:
            return notFound;
        case Response::Status::InternalServerError:
            return internalServerError;
        case Response::Status::NotImplemented:
            return notImplemented;
        case Response::Status::BadGateway:
            return badGateway;
        case Response::Status::ServiceUnavailable:
            return serviceUnavailable;
        default:
            return internalServerError;
    }
}

} // namespace stockReplies

Response Response::stockResponse(Status status)
{
    Response rep;
    rep.status = status;
    rep.content = stockReplies::toString(status);
    rep.headers.resize(2);
    rep.headers[0].name = "Content-Length";
    rep.headers[0].value = boost::lexical_cast<String>(rep.content.size());
    rep.headers[1].name = "Content-Type";
    rep.headers[1].value = "text/html";
    return rep;
}

} // namespace h1::io_server
