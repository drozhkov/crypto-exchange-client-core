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
	struct t_order_update;

	enum class Coin {
		_undef,
		A_UNKNOWN,
		A_ANY,
		A_ALL,
		BTC,
		USDT,
		ETH,
		KCS,
		TRX
	};

	enum class Symbol : size_t { _undef, A_UNKNOWN = 1024, A_ANY, A_ALL };

	enum class Direction { _undef, BUY, SELL };

	using t_number = as::FixedNumber;
	using t_order_id = as::t_string;

	using t_exchangeClientReadyHandler = std::function<void( Client & )>;
	using t_exchangeClientErrorHandler = std::function<void( Client & )>;

	using t_priceBookTickerHandler =
		std::function<void( Client &, t_price_book_ticker & )>;

	using t_orderUpdateHandler =
		std::function<void( Client &, t_order_update & )>;


	struct t_price_book_ticker {
		Symbol symbol;
		t_number bidPrice;
		t_number bidQuantity;
		t_number askPrice;
		t_number askQuantity;
	};

	struct t_order {
		t_order_id id;
	};

	struct t_order_update {
		t_order_id orderId;
	};

	class Pair {
	protected:
		Coin m_base;
		Coin m_quote;
		as::t_string m_name;

	public:
		Pair()
		{
		}

		Pair( Coin base, Coin quote, const as::t_string & name )
			: m_base( base )
			, m_quote( quote )
			, m_name( name )
		{
		}

		Coin Base() const
		{
			return m_base;
		}

		Coin Quote() const
		{
			return m_quote;
		}

		const as::t_string & Name() const
		{
			return m_name;
		}
	};

	class Client {
	protected:
		as::Url m_wsApiUrl;
		as::Url m_httpApiUrl;

		HttpsClient m_httpClient;
		std::unique_ptr<as::WsClient> m_wsClient;
		t_timespan m_wsTimeoutMs{ 0 };

		t_exchangeClientReadyHandler m_clientReadyHandler;
		t_exchangeClientErrorHandler m_clientErrorHandler;

		std::unordered_map<Symbol, t_priceBookTickerHandler>
			m_priceBookTickerHandlerMap;

		t_orderUpdateHandler m_orderUpdateHandler;

		std::unordered_map<as::t_stringview, Coin> m_coinMap;
		std::unordered_map<Coin, as::t_stringview> m_coinReverseMap;

		std::unordered_map<as::t_stringview, Symbol> m_symbolMap;

		std::vector<Pair> m_pairList;

	protected:
		virtual void wsErrorHandler(
			as::WsClient &, int, const as::t_string & ) = 0;

		virtual void wsHandshakeHandler( as::WsClient & ) = 0;
		virtual bool wsReadHandler( as::WsClient &, const char *, size_t ) = 0;

		void addSymbolMapEntry(
			const as::t_stringview & name, as::cryptox::Symbol s )
		{

			m_symbolMap.emplace( name, s );
		}

		void addCoinMapEntry(
			const as::t_stringview & name, as::cryptox::Coin c )
		{

			m_coinMap.emplace( name, c );
			m_coinReverseMap.emplace( c, name );
		}

		virtual void initCoinMap()
		{
			addCoinMapEntry( AS_T( "UNKNOWN" ), Coin::A_UNKNOWN );

			addCoinMapEntry( AS_T( "BTC" ), Coin::BTC );
			addCoinMapEntry( AS_T( "ETH" ), Coin::ETH );
			addCoinMapEntry( AS_T( "KCS" ), Coin::KCS );
			addCoinMapEntry( AS_T( "TRX" ), Coin::TRX );
			addCoinMapEntry( AS_T( "USDT" ), Coin::USDT );
		}

		virtual void initSymbolMap();
		virtual void initWsClient();

		template <typename TMap, typename TArg>
		void callSymbolHandler(
			as::cryptox::Symbol symbol, TMap & map, TArg & arg )
		{

			auto it = map.find( symbol );

			if ( map.end() == it ) {
				it = map.find( Symbol::A_ANY );
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
		}

		virtual ~Client() = default;

		template <typename T> static t_timespan UnixTs()
		{
			return std::chrono::duration_cast<T>(
				std::chrono::system_clock::now().time_since_epoch() )
				.count();
		}

		virtual void run( const t_exchangeClientReadyHandler & handler )
		{
			initCoinMap();
			initSymbolMap();

			m_clientReadyHandler = handler;
		}

		virtual void subscribePriceBookTicker(
			Symbol symbol, const t_priceBookTickerHandler & handler )
		{

			if ( Symbol::A_ALL == symbol ) {
				symbol = Symbol::A_ANY;
			}

			m_priceBookTickerHandlerMap.emplace( symbol, handler );
		}

		virtual void subscribeOrderUpdate(
			const t_orderUpdateHandler & handler )
		{

			m_orderUpdateHandler = handler;
		}

		virtual t_order placeOrder( Direction direction,
			Symbol symbol,
			const FixedNumber & price,
			const FixedNumber & quantity ) = 0;

		virtual const as::t_char * SymbolName( Symbol symbol ) const
		{
			return Pair( symbol ).Name().c_str();
		}

		virtual Symbol Symbol( const as::t_char * symbolName ) const
		{
			auto it = m_symbolMap.find( symbolName );

			if ( m_symbolMap.end() != it ) {
				return it->second;
			}

			return Symbol::A_UNKNOWN;
		}

		virtual const as::t_char * CoinName( Coin coin ) const
		{
			auto it = m_coinReverseMap.find( coin );

			if ( m_coinReverseMap.end() != it ) {
				return it->second.data();
			}

			return CoinName( Coin::A_UNKNOWN );
		}

		virtual Coin Coin( const as::t_char * coinName ) const
		{
			auto it = m_coinMap.find( coinName );

			if ( m_coinMap.end() != it ) {
				return it->second;
			}

			return Coin::A_UNKNOWN;
		}

		virtual const Pair & Pair( as::cryptox::Symbol symbol ) const
		{
			if ( symbol >= as::cryptox::Symbol::A_UNKNOWN ) {
				symbol = as::cryptox::Symbol::_undef;
			}

			return m_pairList[static_cast<size_t>( symbol )];
		}

		virtual const as::t_char * DirectionName( Direction direction ) const
		{
			switch ( direction ) {
				case Direction::BUY:
					return AS_T( "buy" );

				case Direction::SELL:
					return AS_T( "sell" );
			}

			return AS_T( "UNKNOWN" );
		}

		Client & ErrorHandler( const t_exchangeClientErrorHandler & handler )
		{
			m_clientErrorHandler = handler;
			return *this;
		}
	};

}


#endif
