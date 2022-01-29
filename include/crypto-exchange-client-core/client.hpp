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

/// client.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__CLIENT__H


#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

#include "core.hpp"
#include "wsClient.hpp"


namespace as::cryptox {

	class Client;

	using t_exchangeClientReadyHandler = std::function<void( Client & )>;


	class Client {
	protected:
		as::Url m_wsApiUrl;

		std::unique_ptr<as::WsClient> m_wsClient;

		t_exchangeClientReadyHandler m_clientReadyHandler;

	protected:
		virtual void wsErrorHandler(
			as::WsClient &, int, const std::string & ) = 0;

		virtual void wsHandshakeHandler( as::WsClient & ) = 0;
		virtual void wsReadHandler( as::WsClient &, const char *, size_t ) = 0;
		virtual void initWsClient();

	public:
		Client( const as::t_string & wsApiUrl )
			: m_wsApiUrl( wsApiUrl )
		{
		}

		virtual ~Client() = default;

		virtual void run( const t_exchangeClientReadyHandler & handler ) = 0;
	};

}


#endif
