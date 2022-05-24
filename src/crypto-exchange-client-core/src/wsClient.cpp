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

/// wsClient.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "crypto-exchange-client-core/core.hpp"

#include "crypto-exchange-client-core/wsClient.hpp"


namespace as {

	void WsClient::OnResolve( boost::system::error_code ec,
		boost::asio::ip::tcp::resolver::results_type results )
	{

		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}

		boost::asio::async_connect( m_stream.next_layer().next_layer(),
			results.begin(),
			results.end(),
			std::bind( &WsClient::OnConnect,
				this,
				std::placeholders::_1,
				std::placeholders::_2 ) );
	}

	void WsClient::OnConnect( boost::system::error_code ec,
		boost::asio::ip::tcp::resolver::results_type::iterator it )
	{

		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}

		SSL_set_tlsext_host_name(
			m_stream.next_layer().native_handle(), m_url.Hostname().c_str() );

		m_stream.next_layer().async_handshake(
			boost::asio::ssl::stream_base::client,
			std::bind(
				&WsClient::OnSslHandshake, this, std::placeholders::_1 ) );
	}

	void WsClient::OnSslHandshake( boost::system::error_code ec )
	{
		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}

		m_stream.async_handshake( m_url.Hostname(),
			m_url.Path(),
			std::bind( &WsClient::OnHandshake, this, std::placeholders::_1 ) );
	}

	void WsClient::OnHandshake( boost::system::error_code ec )
	{
		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}

		m_stream.control_callback( std::bind( &WsClient::OnControl,
			this,
			std::placeholders::_1,
			std::placeholders::_2 ) );

		AS_CALL( m_handshakeHandler, *this );
	}

	void WsClient::OnWriteComplete(
		boost::system::error_code ec, std::size_t bytesWritten )
	{

		boost::ignore_unused( bytesWritten );

		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}
	}

	void WsClient::OnReadComplete(
		boost::system::error_code ec, std::size_t bytesRead )
	{
		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}

		refreshLastActivityTs();

		try {
			if ( m_readHandler &&
				!m_readHandler( *this,
					static_cast<const char *>( m_buffer.data().data() ),
					m_buffer.data().size() ) ) {

				return;
			}
		}
		catch ( ... ) {
		}

		m_buffer.consume( bytesRead );

		readAsync();
	}

	void WsClient::OnPingComplete( boost::system::error_code ec )
	{
		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}
	}

	void WsClient::OnClose( boost::system::error_code ec )
	{
		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
			return;
		}
	}

	void WsClient::OnControl( boost::beast::websocket::frame_type type,
		boost::beast::string_view payload )
	{

		refreshLastActivityTs();

		if ( boost::beast::websocket::frame_type::ping == type ) {
			m_stream.async_pong(
				"as::wsClient", []( boost::system::error_code ) {} );
		}
	}

	void WsClient::run()
	{
		m_stream.next_layer().set_verify_mode( boost::asio::ssl::verify_none );

		boost::asio::ip::tcp::resolver resolver( m_io );
		std::string portS = std::to_string( m_url.Port() );
		resolver.async_resolve( m_url.Hostname(),
			portS,
			std::bind( &WsClient::OnResolve,
				this,
				std::placeholders::_1,
				std::placeholders::_2 ) );

		refreshLastActivityTs();
		m_isWatchdogActive.test_and_set();

		m_watchdogThread = std::thread( [this] {
			while ( m_isWatchdogActive.test_and_set() ) {
				std::this_thread::sleep_for(
					std::chrono::milliseconds( m_watchdogTimeoutMs / 2 ) );

				if ( NowTs() - m_lastActivityTs > m_watchdogTimeoutMs ) {
					m_io.stop();

					return;
				}
			}
		} );

		m_io.run();
	}

	void WsClient::readAsync()
	{
		m_stream.async_read( m_buffer,
			std::bind( &WsClient::OnReadComplete,
				this,
				std::placeholders::_1,
				std::placeholders::_2 ) );
	}

	void WsClient::write( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamWriteSync );
		boost::system::error_code ec;
		m_stream.write( boost::asio::buffer( data, size ), ec );

		if ( ec ) {
			AS_CALL( m_errorHandler, *this, ec.value(), ec.message() );
		}
	}

	void WsClient::writeAsync( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamWriteSync );
		m_stream.async_write( boost::asio::buffer( data, size ),
			std::bind( &WsClient::OnWriteComplete,
				this,
				std::placeholders::_1,
				std::placeholders::_2 ) );
	}

	void WsClient::pingAsync( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamPingSync );
		m_stream.async_ping( { static_cast<const char *>( data ), size },
			std::bind(
				&WsClient::OnPingComplete, this, std::placeholders::_1 ) );
	}

}
