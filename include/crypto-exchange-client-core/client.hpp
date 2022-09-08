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
#include <initializer_list>
#include <thread>

#include "core.hpp"
#include "logger.hpp"
#include "wsClient.hpp"
#include "httpClient.hpp"


namespace as::cryptox {

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

	enum class Symbol : size_t { _undef = 0, A_UNKNOWN = 4096, A_ANY, A_ALL };

	enum class Direction { _undef, BUY, SELL };

	using t_number = as::FixedNumber;
	using t_order_id = as::t_string;


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
		FixedNumber m_baseMinSize;
		FixedNumber m_quoteMinSize;
		FixedNumber m_baseIncrement;
		FixedNumber m_quoteIncrement;
		FixedNumber m_priceIncrement;

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

		Pair( Coin base,
			Coin quote,
			const as::t_string & name,
			const FixedNumber & baseMinSize,
			const FixedNumber & quoteMinSize,
			const FixedNumber & baseIncrement,
			const FixedNumber & quoteIncrement,
			const FixedNumber & priceIncrement )
			: m_base( base )
			, m_quote( quote )
			, m_name( name )
			, m_baseMinSize( baseMinSize )
			, m_quoteMinSize( quoteMinSize )
			, m_baseIncrement( baseIncrement )
			, m_quoteIncrement( quoteIncrement )
			, m_priceIncrement( priceIncrement )
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

		const FixedNumber & BaseMinSize() const
		{
			return m_baseMinSize;
		}

		const FixedNumber & QuoteMinSize() const
		{
			return m_quoteMinSize;
		}

		const FixedNumber & BaseIncrement() const
		{
			return m_baseIncrement;
		}

		const FixedNumber & QuoteIncrement() const
		{
			return m_quoteIncrement;
		}

		const FixedNumber & PriceIncrement() const
		{
			return m_priceIncrement;
		}
	};

	class Client {
	public:
		/// @brief
		using t_exchangeClientReadyHandler =
			std::function<void( Client &, size_t wsClientIndex )>;

		/// @brief
		using t_exchangeClientErrorHandler =
			std::function<void( Client &, size_t wsClientIndex )>;

		/// @brief
		using t_priceBookTickerHandler = std::function<void(
			Client &, size_t wsClientIndex, t_price_book_ticker & )>;

		/// @brief
		using t_orderUpdateHandler = std::function<void(
			Client &, size_t wsClientIndex, t_order_update & )>;

	protected:
		std::vector<as::Url> m_wsApiUrls;
		std::vector<as::Url> m_httpApiUrls;

		HttpsClient m_httpClient;
		std::vector<std::unique_ptr<as::WsClient>> m_wsClients;
		t_timespan m_wsTimeoutMs{ 0 };
		std::vector<std::thread> m_wsClientsThreads;

		t_exchangeClientReadyHandler m_clientReadyHandler;
		t_exchangeClientErrorHandler m_clientErrorHandler;

		std::unordered_map<Symbol, t_priceBookTickerHandler>
			m_priceBookTickerHandlerMap;

		t_orderUpdateHandler m_orderUpdateHandler;

		std::unordered_map<as::t_stringview, Coin> m_coinMap;
		std::unordered_map<Coin, as::t_stringview> m_coinReverseMap;

		std::unordered_map<as::t_string, Symbol> m_symbolMap;
		std::unordered_map<Coin, std::unordered_map<Coin, Symbol>>
			m_coinSymbolMap;

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

			if ( s < Symbol::A_UNKNOWN ) {
				const auto & pair = m_pairList[static_cast<size_t>( s )];

				if ( pair.Base() != Coin::A_UNKNOWN &&
					pair.Quote() != Coin::A_UNKNOWN ) {

					m_coinSymbolMap[pair.Base()][pair.Quote()] = s;
				}
			}
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
		virtual void initWsClient( size_t index );

		template <typename TMap, typename TArg>
		void callSymbolHandler(
			as::cryptox::Symbol symbol, TMap & map, size_t index, TArg & arg )
		{

			auto it = map.find( symbol );

			if ( map.end() == it ) {
				it = map.find( Symbol::A_ANY );
			}

			if ( map.end() != it ) {
				it->second( *this, index, arg );
			}
		}

		template <typename TResult, typename TFunc>
		std::pair<bool, TResult> callWsClient(
			const size_t index, const TFunc & func )
		{

			if ( !m_wsClients[index] || !m_wsClients[index]->IsOpen() ) {
				return ( std::pair<bool, TResult>( false, TResult() ) );
			}

			return ( std::pair<bool, TResult>(
				true, func( m_wsClients[index].get() ) ) );
		}

	public:
		Client( const std::initializer_list<as::t_string> & httpApiUrls,
			const std::initializer_list<as::t_string> & wsApiUrls )
		{
			for ( const auto & httpApiUrl : httpApiUrls ) {
				m_httpApiUrls.emplace_back( httpApiUrl );
			}

			for ( const auto & wsApiUrl : wsApiUrls ) {
				m_wsApiUrls.emplace_back( wsApiUrl );
			}

			m_wsClients.resize( wsApiUrls.size() );
			m_wsClientsThreads.resize( wsApiUrls.size() );
		}

		virtual ~Client() = default;

		template <typename T> static t_timespan UnixTs()
		{
			return std::chrono::duration_cast<T>(
				std::chrono::system_clock::now().time_since_epoch() )
				.count();
		}

		/// @brief
		/// @param handler
		virtual void run(
			const t_exchangeClientReadyHandler & handler,
			const std::function<void( size_t )> & beforeRun = []( size_t ) {} );

		/// @brief
		/// @param symbol
		/// @param handler
		virtual bool subscribePriceBookTicker( size_t wsClientIndex,
			Symbol symbol,
			const t_priceBookTickerHandler & handler )
		{

			if ( Symbol::A_ALL == symbol ) {
				symbol = Symbol::A_ANY;
			}

			return (
				m_priceBookTickerHandlerMap.emplace( symbol, handler ).second );
		}

		/// @brief
		/// @param handler
		virtual void subscribeOrderUpdate(
			size_t wsClientIndex, const t_orderUpdateHandler & handler )
		{

			m_orderUpdateHandler = handler;
		}

		/// @brief
		/// @param direction
		/// @param symbol
		/// @param price
		/// @param quantity
		/// @return
		virtual t_order placeOrder( Direction direction,
			Symbol symbol,
			const FixedNumber & price,
			const FixedNumber & quantity ) = 0;

		/// @brief
		/// @param symbol
		/// @return
		virtual const as::t_char * toName( Symbol symbol ) const
		{
			if ( Symbol::A_ALL == symbol ) {
				return AS_T( "all" );
			}

			return toPair( symbol ).Name().c_str();
		}

		/// @brief
		/// @param symbolName
		/// @return
		virtual ::as::cryptox::Symbol toSymbol( const as::t_char * symbolName )
		{
			auto it = m_symbolMap.find( symbolName );

			if ( m_symbolMap.end() != it ) {
				return it->second;
			}

			return ::as::cryptox::Symbol::A_UNKNOWN;
		}

		virtual ::as::cryptox::Symbol toSymbol( Coin base, Coin quote )
		{
			auto s = m_coinSymbolMap[base][quote];

			return (
				s == Symbol::_undef ? ::as::cryptox::Symbol::A_UNKNOWN : s );
		}

		/// @brief
		/// @param coin
		/// @return
		virtual const as::t_char * toName( Coin coin ) const
		{
			auto it = m_coinReverseMap.find( coin );

			if ( m_coinReverseMap.end() != it ) {
				return it->second.data();
			}

			return toName( Coin::A_UNKNOWN );
		}

		/// @brief
		/// @param coinName
		/// @return
		virtual ::as::cryptox::Coin toCoin( const as::t_char * coinName ) const
		{
			auto it = m_coinMap.find( coinName );

			if ( m_coinMap.end() != it ) {
				return it->second;
			}

			return ::as::cryptox::Coin::A_UNKNOWN;
		}

		/// @brief
		/// @param symbol
		/// @return
		virtual const ::as::cryptox::Pair & toPair(
			as::cryptox::Symbol symbol ) const
		{

			if ( symbol >= as::cryptox::Symbol::A_UNKNOWN ) {
				symbol = as::cryptox::Symbol::_undef;
			}

			return m_pairList[static_cast<size_t>( symbol )];
		}

		/// @brief
		/// @param direction
		/// @return
		virtual const as::t_char * toName( Direction direction ) const
		{
			switch ( direction ) {
				case Direction::BUY:
					return AS_T( "buy" );

				case Direction::SELL:
					return AS_T( "sell" );
			}

			return AS_T( "UNKNOWN" );
		}

		/// @brief
		/// @param handler
		/// @return
		Client & ErrorHandler( const t_exchangeClientErrorHandler & handler )
		{
			m_clientErrorHandler = handler;
			return *this;
		}
	};

} // namespace as::cryptox


#endif
