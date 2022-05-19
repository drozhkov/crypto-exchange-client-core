/*
MIT License
Copyright (c) 2022 Denis Rozhkov <denis@rozhkoff.com>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/// httpClient.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "boost/beast/core/buffers_to_string.hpp"

#include "crypto-exchange-client-core/httpClient.hpp"


namespace as {

	void PersistentHttpsClient::connect()
	{
		if ( m_isConnected.test_and_set() ) {
			return;
		}

		// TODO
		m_ctx.set_verify_mode( boost::asio::ssl::verify_none );

		boost::asio::ip::tcp::resolver resolver( m_ioc );

		SSL_set_tlsext_host_name(
			m_stream.native_handle(), m_hostname.c_str() );

		auto results = resolver.resolve( m_hostname, std::to_string( m_port ) );
		boost::asio::connect(
			m_stream.next_layer(), results.begin(), results.end() );

		m_stream.handshake( boost::asio::ssl::stream_base::client );
	}

	HttpResponse PersistentHttpsClient::request( const Url & uri,
		const HttpHeaderList & headers,
		boost::beast::http::verb verb,
		const as::t_stringview & body )
	{

		connect();

		boost::beast::http::request<boost::beast::http::string_body> req(
			verb, uri.Path(), HttpVersion );

		req.set( boost::beast::http::field::host, m_hostname );
		req.set( boost::beast::http::field::user_agent, m_userAgent );

		for ( size_t i = 0; i < headers.Count(); ++i ) {
			auto & header = headers.Item( i );
			req.set( header.Name(), header.Value() );
		}

		if ( !body.empty() ) {
			req.body() = body;
			req.prepare_payload();
		}

		{
			std::lock_guard<std::mutex> lock( m_streamSync );

			boost::beast::error_code ec;
			boost::beast::http::write( m_stream, req, ec );

			boost::beast::flat_buffer buffer;
			boost::beast::http::response<boost::beast::http::dynamic_body> res;
			boost::beast::http::read( m_stream, buffer, res, ec );

			if ( ec ) {
				return HttpResponse( ec.value() == 1 );
			}

			return HttpResponse(
				boost::beast::buffers_to_string( res.body().data() ) );
		}
	}

	HttpResponse PersistentHttpsClient::get(
		const Url & uri, const HttpHeaderList & headers )
	{

		return request( uri, headers, boost::beast::http::verb::get );
	}

	HttpResponse PersistentHttpsClient::post( const Url & uri,
		const HttpHeaderList & headers,
		const as::t_stringview & body )
	{

		return request( uri, headers, boost::beast::http::verb::post, body );
	}

	HttpResponse PersistentHttpsClient::put( const Url & uri,
		const HttpHeaderList & headers,
		const as::t_stringview & body )
	{

		return request( uri, headers, boost::beast::http::verb::put, body );
	}

	//

	std::shared_ptr<PersistentHttpsClient> HttpsClient::persistentClient(
		const as::t_stringview & hostname, bool forceNew )
	{

		std::lock_guard<std::recursive_mutex> lock(
			m_persistentClientsMapSync );

		auto it = forceNew ? m_persistentClientsMap.end()
						   : m_persistentClientsMap.find( hostname.data() );

		if ( m_persistentClientsMap.end() == it ) {
			it = m_persistentClientsMap
					 .insert_or_assign( hostname.data(),
						 std::make_shared<PersistentHttpsClient>( hostname ) )
					 .first;
		}

		return ( it->second );
	}

	as::t_string HttpsClient::get(
		const Url & uri, const HttpHeaderList & headers )
	{

		return makeRequest( uri, [&uri, &headers]( auto & client ) {
			return client->get( uri, headers );
		} );
	}

	as::t_string HttpsClient::post( const Url & uri,
		const HttpHeaderList & headers,
		const as::t_stringview & body )
	{

		return makeRequest( uri, [&uri, &headers, &body]( auto & client ) {
			return client->post( uri, headers, body );
		} );
	}

	as::t_string HttpsClient::put( const Url & uri,
		const HttpHeaderList & headers,
		const as::t_stringview & body )
	{

		return makeRequest( uri, [&uri, &headers, &body]( auto & client ) {
			return client->put( uri, headers, body );
		} );
	}

}
