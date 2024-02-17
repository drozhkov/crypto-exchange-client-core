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

/// wsClient.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__WS_CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__WS_CLIENT__H


#include <mutex>
#include <atomic>
#include <thread>

#include "boost/asio/connect.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/ssl/stream.hpp"

#include "boost/beast/core.hpp"
#include "boost/beast/websocket.hpp"
#include "boost/beast/websocket/ssl.hpp"

#include "core.hpp"
#include "url.hpp"


namespace as {

	class WsClient;

	using t_wsErrorHandler =
		std::function<void( WsClient &, int, const std::string & )>;

	using t_wsHandshakeHandler = std::function<void( WsClient & )>;

	using t_wsReadHandler =
		std::function<bool( WsClient &, const char *, size_t )>;

	using t_wsStream = boost::beast::websocket::stream<
		boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>;


	class WsClient {
	protected:
		t_timespan m_watchdogTimeoutMs = 15 * 1000;

		Url m_url;
		size_t m_index;

		boost::asio::io_context m_io;
		boost::asio::ssl::context m_ctx;

		t_wsStream m_stream;

		std::mutex m_streamWriteSync;
		std::mutex m_streamPingSync;

		boost::beast::flat_buffer m_buffer;

		t_wsErrorHandler m_errorHandler;
		t_wsHandshakeHandler m_handshakeHandler;
		t_wsReadHandler m_readHandler;

		std::atomic_int64_t m_lastActivityTs;

		std::thread m_watchdogThread;
		std::atomic_flag m_isWatchdogActive;

		std::thread m_pingThread;
		std::atomic_flag m_isPingActive;

	protected:
		static auto NowTs()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::steady_clock::now().time_since_epoch() )
				.count();
		}

		void refreshLastActivityTs()
		{
			m_lastActivityTs.store( NowTs() );
		}

		void OnResolve( boost::system::error_code ec,
			boost::asio::ip::tcp::resolver::results_type results );

		void OnConnect( boost::system::error_code ec,
			boost::asio::ip::tcp::resolver::results_type::iterator it );

		void OnSslHandshake( boost::system::error_code ec );
		void OnHandshake( boost::system::error_code ec );

		void OnWriteComplete(
			boost::system::error_code ec, std::size_t bytesWritten );

		void OnReadComplete(
			boost::system::error_code ec, std::size_t bytesRead );

		void OnPingComplete( boost::system::error_code ec );

		void OnControl(
			boost::beast::websocket::frame_type, boost::beast::string_view );

		void OnClose( boost::system::error_code ec );

	public:
		WsClient( const Url & url, size_t index )
			: m_url( url )
			, m_index( index )
			, m_ctx( boost::asio::ssl::context::method::tls_client )
			, m_stream( m_io, m_ctx )
		{
		}

		virtual ~WsClient()
		{
			m_isWatchdogActive.clear();
			m_isPingActive.clear();

			if ( m_watchdogThread.joinable() ) {
				m_watchdogThread.join();
			}

			if ( m_pingThread.joinable() ) {
				m_pingThread.join();
			}
		}

		void run();
		void readAsync();
		void write( const void * data, size_t size );
		void writeAsync( const void * data, size_t size );
		void pingAsync( const void * data, size_t size );

		template <typename F, typename R, typename P>
		void startPing( const F & ping,
			const std::chrono::duration<R, P> & interval )
		{

			if ( m_pingThread.joinable() ) {
				return;
			}

			m_isPingActive.test_and_set();

			m_pingThread = std::thread( [this, ping, interval] {
				while ( m_isPingActive.test_and_set() ) {
					std::this_thread::sleep_for( interval );
					ping( *this );
				}
			} );
		}

		void ErrorHandler( const t_wsErrorHandler & handler )
		{
			m_errorHandler = handler;
		}

		void HandshakeHandler( const t_wsHandshakeHandler & handler )
		{
			m_handshakeHandler = handler;
		}

		void ReadHandler( const t_wsReadHandler & handler )
		{
			m_readHandler = handler;
		}

		void WatchdogTimeoutMs( t_timespan t )
		{
			if ( t != 0 ) {
				m_watchdogTimeoutMs = t;
			}
		}

		size_t Index() const
		{
			return m_index;
		}

		bool IsOpen() const
		{
			return m_stream.is_open();
		}
	};

} // namespace as


#endif
