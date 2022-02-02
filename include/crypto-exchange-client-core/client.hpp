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
#include "httpClient.hpp"


namespace as::cryptox {

	class Client;
	struct t_price_book_ticker;

	enum class Symbol { _undef, UNKNOWN, ANY, ALL, BTC_USDT };

	using t_number = as::FixedNumber;

	using t_exchangeClientReadyHandler = std::function<void( Client & )>;
	using t_priceBookTickerHandler =
		std::function<void( Client &, t_price_book_ticker & )>;


	struct t_price_book_ticker {
		Symbol symbol;
		t_number bidPrice;
		t_number bidQuantity;
		t_number askPrice;
		t_number askQuantity;
	};

	class Client {
	protected:
		as::Url m_wsApiUrl;
		as::Url m_httpApiUrl;

		HttpsClient m_httpClient;
		std::unique_ptr<as::WsClient> m_wsClient;
		t_timespan m_wsTimeoutMs;

		t_exchangeClientReadyHandler m_clientReadyHandler;

		std::unordered_map<Symbol, t_priceBookTickerHandler>
			m_priceBookTickerHandlerMap;

		std::unordered_map<as::t_stringview, Symbol> m_symbolMap;
		std::unordered_map<Symbol, as::t_stringview> m_symbolReverseMap;

	protected:
		template <typename T> static t_timespan UnixTs()
		{
			return std::chrono::duration_cast<T>(
				std::chrono::system_clock::now().time_since_epoch() )
				.count();
		}

		virtual void wsErrorHandler(
			as::WsClient &, int, const as::t_string & ) = 0;

		virtual void wsHandshakeHandler( as::WsClient & ) = 0;
		virtual void wsReadHandler( as::WsClient &, const char *, size_t ) = 0;

		void addSymbolMapEntry(
			const as::t_stringview & name, as::cryptox::Symbol s )
		{

			m_symbolMap.emplace( name, s );
			m_symbolReverseMap.emplace( s, name );
		}

		virtual void initSymbolMap()
		{
			addSymbolMapEntry( AS_T( "all" ), as::cryptox::Symbol::ALL );
		}

		virtual void initWsClient();

		template <typename TMap, typename TArg>
		void callSymbolHandler(
			as::cryptox::Symbol symbol, TMap & map, TArg & arg )
		{

			auto it = map.find( symbol );

			if ( map.end() == it ) {
				it = map.find( Symbol::ANY );
			}

			if ( map.end() != it ) {
				it->second( *this, arg );
			}
		}

	public:
		Client( const as::t_string & httpApiUrl, const as::t_string & wsApiUrl )
			: m_httpApiUrl( httpApiUrl )
			, m_wsApiUrl( wsApiUrl )
		{
			initSymbolMap();
		}

		virtual ~Client() = default;

		virtual void run( const t_exchangeClientReadyHandler & handler ) = 0;

		virtual void subscribePriceBookTicker(
			Symbol symbol, const t_priceBookTickerHandler & handler )
		{

			if ( Symbol::ALL == symbol ) {
				symbol = Symbol::ANY;
			}

			m_priceBookTickerHandlerMap.emplace( symbol, handler );
		}

		virtual const as::t_char * SymbolName( Symbol symbol ) const
		{
			auto it = m_symbolReverseMap.find( symbol );

			if ( m_symbolReverseMap.end() != it ) {
				return it->second.data();
			}

			return AS_T( "UNKNOWN" );
		}

		virtual Symbol Symbol( const as::t_char * symbolName ) const
		{
			auto it = m_symbolMap.find( symbolName );

			if ( m_symbolMap.end() != it ) {
				return it->second;
			}

			return Symbol::UNKNOWN;
		}
	};

}


#endif
