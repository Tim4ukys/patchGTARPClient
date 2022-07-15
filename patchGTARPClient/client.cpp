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

std::array<std::string, 3U> sortURLtext(std::string text) {
    //const std::regex r{R"((http|https):\/\/(\w+[\.\w+]+)([\/\w+]+))"};
    const std::regex r{R"((http|https):\/\/([\w-_]+[\.\w+]+)([-a-zA-Z0-9()@:%_\+.~#?&//=\"\']*))"};
    std::smatch      m;

    std::array<std::string, 3U> rr;
    // auto                        data_rr = rr.data();

    if (std::regex_search(text, m, r)) {
        for (size_t i{0}; i < 3;) {
            rr[i++] = m[i + 1];
        }
    }
    return rr;
}

std::string client::downloadStringFromURL(std::string URL) {
    const auto& [port, host, localPath] = sortURLtext(URL);

    std::string             responce;
    boost::asio::io_service io_service; // io_context
    using t_tcp = boost::asio::ip::tcp;

    if (!strcmp("https", port.c_str())) {
        t_tcp::resolver                         resolver(io_service);
        boost::asio::ssl::context               ctx(boost::asio::ssl::context::method::sslv23_client);
        boost::asio::ssl::stream<t_tcp::socket> ssock(io_service, ctx);

        boost::asio::connect(ssock.lowest_layer(), resolver.resolve(host, port));
        ssock.handshake(boost::asio::ssl::stream_base::handshake_type::client);

        {
            std::stringstream request;
            request <<  "GET " << localPath << " HTTP/1.1\r\n"
                        "Host: " << host << "\r\n"
                        "User-Agent: Mozilla/5.0\r\n"
                        "Connection: close\r\n\r\n";

            boost::asio::write(ssock, boost::asio::buffer(request.str()));

            boost::beast::flat_buffer                                      buffer;
            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            boost::beast::http::read(ssock, buffer, res);

            responce = boost::beast::buffers_to_string(res.body().data());
        }
    } else {
        t_tcp::resolver resolver(io_service);
        t_tcp::socket   socket(io_service);
        boost::asio::connect(socket, resolver.resolve(host, port));

        {
            std::string request = "GET " + localPath + " HTTP/1.0\r\nConnection: close\r\n\r\n";
            boost::asio::write(socket, boost::asio::buffer(request));

            boost::beast::flat_buffer                                      buffer;
            boost::beast::http::response<boost::beast::http::dynamic_body> res;
            boost::beast::http::read(socket, buffer, res);

            responce = boost::beast::buffers_to_string(res.body().data());
        }
        socket.close();
    }

    return responce;
}