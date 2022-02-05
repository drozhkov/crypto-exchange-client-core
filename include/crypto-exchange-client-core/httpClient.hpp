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

/// httpClient.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__HTTP_CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__HTTP_CLIENT__H


#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <atomic>
#include <vector>

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"

#include "core.hpp"
#include "url.hpp"


namespace as {

	enum class HttpMethod { _undef, GET, POST, PUT };

	class HttpHeader {
	protected:
		std::string m_name;
		std::string m_value;

	public:
		HttpHeader(
			const as::t_stringview & name, const as::t_stringview & value )
			: m_name( name )
			, m_value( value )
		{
		}

		const std::string & Name() const
		{
			return m_name;
		}

		const std::string & Value() const
		{
			return m_value;
		}
	};

	class HttpHeaderList {
	protected:
		std::vector<HttpHeader> m_list;

	public:
		void add(
			const as::t_stringview & name, const as::t_stringview & value )
		{

			m_list.emplace_back( name, value );
		}

		size_t Count() const
		{
			return m_list.size();
		}

		const HttpHeader & Item( size_t index ) const
		{
			return m_list[index];
		}
	};

	class HttpResponse {
	protected:
		bool m_isEos = false;
		as::t_string m_text;

	public:
		HttpResponse( bool isEos )
			: m_isEos( isEos )
		{
		}

		HttpResponse( as::t_string & text )
			: m_text( std::move( text ) )
		{
		}

		bool IsEos() const
		{
			return m_isEos;
		}

		as::t_string & Text()
		{
			return m_text;
		}
	};

	class PersistentHttpsClient {
	protected:
		constexpr static int HttpVersion = 11;

		std::string m_hostname;
		uint16_t m_port;

		std::string m_userAgent;

		boost::asio::io_context m_ioc;
		boost::asio::ssl::context m_ctx;

		boost::asio::ssl::stream<boost::asio::ip::tcp::socket> m_stream;
		std::mutex m_streamSync;

		std::atomic_flag m_isConnected;

	public:
		PersistentHttpsClient(
			const as::t_stringview & hostname, uint16_t port = 443 )
			: m_hostname( hostname )
			, m_port( port )
			, m_userAgent( "as-http-client" )
			, m_ctx( boost::asio::ssl::context::tls_client )
			, m_stream( m_ioc, m_ctx )
		{
		}

		virtual ~PersistentHttpsClient()
		{
			boost::system::error_code ec;
			m_stream.shutdown( ec );
		}

		void connect();

		HttpResponse request( const Url & uri,
			const HttpHeaderList & headers,
			boost::beast::http::verb verb,
			const as::t_stringview & body = "" );

		HttpResponse get( const Url & uri, const HttpHeaderList & headers );
		HttpResponse post( const Url & uri,
			const HttpHeaderList & headers,
			const as::t_stringview & body );

		HttpResponse put( const Url & uri,
			const HttpHeaderList & headers,
			const as::t_stringview & body );
	};

	class HttpsClient {
	protected:
		std::unordered_map<std::string, std::shared_ptr<PersistentHttpsClient>>
			m_persistentClientsMap;

		std::recursive_mutex m_persistentClientsMapSync;

	protected:
		std::shared_ptr<PersistentHttpsClient> persistentClient(
			const as::t_stringview & hostname, bool forceNew = false );

	protected:
		template <typename F>
		as::t_string makeRequest( const Url & uri, const F & f )
		{
			bool forceNew = false;

			while ( true ) {
				auto httpsClient = persistentClient( uri.Hostname(), forceNew );
				auto response = f( httpsClient );

				if ( !response.IsEos() ) {
					return std::move( response.Text() );
				}

				forceNew = true;
			}
		}

	public:
		static const as::t_char * MethodName( as::HttpMethod method )
		{
			switch ( method ) {
				case as::HttpMethod::GET:
					return AS_T( "GET" );
					break;

				case as::HttpMethod::POST:
					return AS_T( "POST" );
					break;

				case as::HttpMethod::PUT:
					return AS_T( "PUT" );
					break;

					// case as::HttpMethod::DELETE:
					//	return AS_T( "DELETE" );
					//	break;
			}

			return AS_T( "UNKNOWN" );
		}

		as::t_string get( const Url & uri, const HttpHeaderList & headers );
		as::t_string post( const Url & uri,
			const HttpHeaderList & headers,
			const as::t_stringview & body = AS_T( "" ) );

		as::t_string put( const Url & uri,
			const HttpHeaderList & headers,
			const as::t_stringview & body = AS_T( "" ) );
	};

}


#endif
