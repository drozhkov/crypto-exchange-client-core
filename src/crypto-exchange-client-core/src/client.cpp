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

/// client.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "crypto-exchange-client-core/client.hpp"


namespace as::cryptox {

	void Client::initSymbolMap()
	{
		addSymbolMapEntry( AS_T( "all" ), as::cryptox::Symbol::A_ALL );
	}

	void Client::initWsClient( size_t index )
	{
		m_wsClients[index] =
			std::make_unique<as::WsClient>( m_wsApiUrls[index], index );

		m_wsClients[index]->ErrorHandler( std::bind( &Client::wsErrorHandler,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3 ) );

		m_wsClients[index]->HandshakeHandler( std::bind(
			&Client::wsHandshakeHandler, this, std::placeholders::_1 ) );

		m_wsClients[index]->ReadHandler( std::bind( &Client::wsReadHandler,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3 ) );

		m_wsClients[index]->WatchdogTimeoutMs( m_wsTimeoutMs );
	}

	void Client::run( const t_exchangeClientReadyHandler & handler,
		const std::function<void( size_t )> & beforeRun )
	{

		initCoinMap();
		initSymbolMap();

		m_clientReadyHandler = handler;

		for ( size_t i = 0; i < m_wsApiUrls.size(); ++i ) {
			std::thread t( [this, i, beforeRun] {
				while ( true ) {
					try {
						initWsClient( i );
						beforeRun( i );
						m_wsClients[i]->run();
					}
					catch ( const std::exception & x ) {
						AS_LOG_ERROR_LINE( x.what() );
					}
				}
			} );

			m_wsClientsThreads[i].swap( t );
		}

		for ( size_t i = 0; i < m_wsClientsThreads.size(); ++i ) {
			m_wsClientsThreads[i].join();
		}
	}

} // namespace as::cryptox
