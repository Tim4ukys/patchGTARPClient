/****************************************************
*                                                   *
*    Developer: Tim4ukys                            *
*                                                   *
*    email: tim4ukys.dev@yandex.ru                  *
*    vk: vk.com/tim4ukys                            *
*                                                   *
*    License: GNU GPLv3                             *
*                                                   *
****************************************************/
#include "pch.h"
#include "client.h"

std::array<std::string, 3U> client::sortURLtext(std::string URL) {
    //const std::regex r{R"((http|https):\/\/(\w+[\.\w+]+)([\/\w+]+))"};
    const std::regex r{R"((http|https):\/\/([\w-_]+[\.\w+]+)([-a-zA-Z0-9()@:%_\+.~#?&//=\"\']*))"};
    std::smatch      m;

    std::array<std::string, 3U> rr;
    // auto                        data_rr = rr.data();

    if (std::regex_search(URL, m, r)) {
        for (size_t i{0}; i < 3;) {
            rr[i++] = m[i + 1];
        }
    }
    return rr;
}

std::string client::downloadStringFromURL(std::string_view URL) {
    const auto [port, host, localPath] = sortURLtext(URL.data());

    using namespace boost::asio;
    namespace http = boost::beast::http;

    std::string responce;
    io_service io_service; // io_context
    using t_tcp = ip::tcp;

    if (!strcmp("https", port.c_str())) {
        t_tcp::resolver            resolver(io_service);
        ssl::context               ctx(ssl::context::method::sslv23_client);
        ssl::stream<t_tcp::socket> ssock(io_service, ctx);

        connect(ssock.lowest_layer(), resolver.resolve(host, port));
        ssock.handshake(ssl::stream_base::handshake_type::client);

        http::request<http::string_body> req{http::verb::get, localPath, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");

        http::write(ssock, req);

        boost::beast::flat_buffer          buffer;
        http::response<http::dynamic_body> res;
        http::read(ssock, buffer, res);

        responce = boost::beast::buffers_to_string(res.body().data());
    } else {
        t_tcp::resolver resolver(io_service);
        t_tcp::socket   socket(io_service);
        connect(socket, resolver.resolve(host, port));

        http::request<http::string_body> req{http::verb::get, localPath, 10};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");
        http::write(socket, req);

        boost::beast::flat_buffer                        buffer;
        http::response<boost::beast::http::dynamic_body> res;
        http::read(socket, buffer, res);

        responce = boost::beast::buffers_to_string(res.body().data());
    }

    return responce;
}