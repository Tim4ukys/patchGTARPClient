/********************************************************
*                                                       *
*   Greets:                                             *
*    m0d_s0beit: FYP, NarutoUA, we_sux and other...     *
*    SA-MP api: imring, FYP, LUCHARE.                   *
*    other: SR_Team, KiN4StAt, DarkP1xel                *
*                                                       *
*   Credits: Tim4ukys. All rights reserved.             *
*                                                       *
********************************************************/
#ifndef _CCLIENT_HPP_
#define _CCLIENT_HPP_

#include <string>
#include <array>

#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

class Client {
public: 
	static inline std::string downloadStringFromURL(bool isHTTPS, std::string host, std::string local_addres)
	{
		using namespace std;
		
		const std::array<string, 2> arrProtocols{ "80"/*http*/, "443"/*https*/ };

		string responce{};
		if (isHTTPS)
		{
			boost::asio::io_service svc;
			boost::asio::ssl::context ctx(boost::asio::ssl::context::method::sslv23_client);
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssock(svc, ctx);

			boost::asio::ip::tcp::resolver resolver(svc);
			boost::asio::connect(ssock.lowest_layer(), resolver.resolve(host, arrProtocols.at(int(isHTTPS))));

			// and the rest unaltered
			ssock.handshake(boost::asio::ssl::stream_base::handshake_type::client);

			std::string request("GET " + local_addres + " HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");
			boost::asio::write(ssock, boost::asio::buffer(request));
			{
				boost::beast::flat_buffer buffer;
				boost::beast::http::response< boost::beast::http::dynamic_body > res;
				boost::beast::http::read(ssock, buffer, res);

				responce = boost::beast::buffers_to_string(res.body().data());
			}
			ssock.shutdown(); ssock.lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		}
		else
		{
			boost::asio::io_context io;
			boost::asio::ip::tcp::resolver resolver(io);
			boost::asio::ip::tcp::socket socket(io);
			boost::asio::connect(socket, resolver.resolve(host, arrProtocols.at(int(isHTTPS))));

			std::string request("GET " + local_addres + " HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");
			boost::asio::write(socket, boost::asio::buffer(request));
			{
				boost::beast::flat_buffer buffer;
				boost::beast::http::response< boost::beast::http::dynamic_body > res;
				boost::beast::http::read(socket, buffer, res);

				responce = boost::beast::buffers_to_string(res.body().data());
			}
			socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		}
		return responce;
	}
};



#endif