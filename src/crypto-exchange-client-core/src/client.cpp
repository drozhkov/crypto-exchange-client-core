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

	void Client::initWsClient()
	{
		m_wsClient = std::make_unique<as::WsClient>( m_wsApiUrl );
		m_wsClient->ErrorHandler( std::bind( &Client::wsErrorHandler,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3 ) );

		m_wsClient->HandshakeHandler( std::bind(
			&Client::wsHandshakeHandler, this, std::placeholders::_1 ) );

		m_wsClient->ReadHandler( std::bind( &Client::wsReadHandler,
			this,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3 ) );

		m_wsClient->WatchdogTimeoutMs( m_wsTimeoutMs );
	}

}
