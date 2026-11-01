#pragma once
#ifndef H_H1_IO_SERVER_RESPONSE_d4f8a2b6e1c34f9da5b7e0c2f8d6a391
#define H_H1_IO_SERVER_RESPONSE_d4f8a2b6e1c34f9da5b7e0c2f8d6a391

#include "h1/common.hpp"
#include "h1/io_server/header.hpp"

#include <boost/asio.hpp>

#include <vector>

namespace h1::io_server
{

/// HTTP-ответ, отправляемый клиенту.
struct Response
{
    /// Статус ответа
    enum class Status
    {
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        MultipleChoices = 300,
        MovedPermanently = 301,
        MovedTemporarily = 302,
        NotModified = 304,
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503
    };

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    Status status;

    /// Заголовки, включаемые в ответ
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::vector<Header> headers;

    /// Содержимое ответа
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    String content;

    /// Преобразовать ответ в набор буферов. Буферы не владеют памятью,
    /// поэтому объект ответа должен оставаться валидным и неизменным,
    /// пока операция записи не завершится.
    std::vector<boost::asio::const_buffer> toBuffers();

    /// Создать стандартный ответ по статусу
    static Response stockResponse(Status status);
};

} // namespace h1::io_server

#endif
