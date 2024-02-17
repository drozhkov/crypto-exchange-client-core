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

/// core.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__CORE__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__CORE__H


#include <string>
#include <string_view>
#include <cmath>
#include <vector>
#include <atomic>

#include "openssl/evp.h"
#include "openssl/hmac.h"

#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/uuid_io.hpp"


namespace as {

#define AS_T( a_t ) a_t
#define AS_STOI std::stoi
#define AS_STOLL std::stoll
#define AS_TOSTRING std::to_string
#define AS_CALL( a, ... )                                                      \
	if ( a ) {                                                                 \
		a( __VA_ARGS__ );                                                      \
	}


	using t_string = std::string;
	using t_char = char;
	using t_stringview = std::string_view;
	using t_byte = unsigned char;
	using t_timespan = int64_t;


	struct t_buffer {
		const t_byte * ptr;
		size_t len;

	public:
		t_buffer( const void * ptr, size_t len )
			: ptr( static_cast<const t_byte *>( ptr ) )
			, len( len )
		{
		}
	};

	/// <summary>
	///
	/// </summary>
	class Spinlock {
	protected:
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
		size_t m_cycleCount;

	public:
		~Spinlock()
		{
			unlock();
		}

		void lock()
		{
			m_cycleCount = 0;

			while ( m_lock.test_and_set() ) {
				m_cycleCount++;
			}
		}

		void unlock()
		{
			m_lock.clear();
		}
	};

	/// <summary>
	///
	/// </summary>
	/// <param name="lock"></param>
	class SpinlockGuard {
		Spinlock & m_lock;

	public:
		SpinlockGuard( Spinlock & lock )
			: m_lock( lock )
		{
			m_lock.lock();
		}

		~SpinlockGuard()
		{
			m_lock.unlock();
		}
	};

	// TODO
	class FixedNumber {
	protected:
		inline static int64_t s_denominators[] = { INT64_C( 1 ),
			INT64_C( 1'0 ),
			INT64_C( 1'0'0 ),
			INT64_C( 1'0'0'0 ),
			INT64_C( 1'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0'0'0'0'0 ),
			INT64_C( 1'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0 ) };

		int64_t m_numerator{ INT64_C( 0 ) };
		int64_t m_denominator{ INT64_C( 0 ) };
		size_t m_exponent{ 0 };

	public:
		FixedNumber()
		{
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="s"></param>
		FixedNumber( const as::t_stringview & s )
		{
			Value( s );
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="s"></param>
		void Value( const as::t_stringview & s )
		{
			auto dotPos = s.find( AS_T( '.' ) );
			auto i =
				as::t_stringview::npos == dotPos ? s : s.substr( 0, dotPos );

			auto f = as::t_stringview::npos == dotPos ? AS_T( "" )
													  : s.substr( dotPos + 1 );

			m_numerator = AS_STOLL( as::t_string( i ) + as::t_string( f ) );
			m_exponent = f.length();
			m_denominator = s_denominators[m_exponent];
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="v"></param>
		void Value( double v )
		{
			m_exponent = 8;
			m_denominator = s_denominators[m_exponent];
			m_numerator = static_cast<int64_t>( v * m_denominator );
		}

		/// <summary>
		///
		/// </summary>
		/// <returns></returns>
		as::t_string toString() const
		{
			if ( INT64_C( 0 ) == m_denominator ) {
				return AS_T( "NaN" );
			}

			auto n = AS_TOSTRING( m_numerator );

			if ( n.length() <= m_exponent ) {
				n = as::t_string( m_exponent - n.length() + 1, AS_T( '0' ) ) +
					n;
			}

			auto i = AS_TOSTRING( m_numerator / m_denominator );

			return ( i + AS_T( '.' ) + n.substr( i.length() ) );
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="d"></param>
		/// <returns></returns>
		FixedNumber div( double d ) const
		{
			FixedNumber result;
			result.m_numerator = static_cast<int64_t>( m_numerator / d );
			result.m_exponent = m_exponent;
			result.m_denominator = m_denominator;

			return result;
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="s"></param>
		/// <returns></returns>
		FixedNumber sub( double s ) const
		{
			FixedNumber result;
			result.m_numerator =
				m_numerator - static_cast<int64_t>( s * m_denominator );

			result.m_exponent = m_exponent;
			result.m_denominator = m_denominator;

			return result;
		}

		/// <summary>
		///
		/// </summary>
		/// <returns></returns>
		double Value() const
		{
			return ( static_cast<double>( m_numerator ) / m_denominator );
		}

		bool IsZero() const
		{
			return ( m_numerator == INT64_C( 0 ) );
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="l"></param>
		/// <param name="r"></param>
		/// <returns></returns>
		bool operator<( const FixedNumber & r )
		{
			return ( Value() < r.Value() );
		}

		///
		friend bool operator<( const FixedNumber & l, const FixedNumber & r )
		{
			return ( l.Value() < r.Value() );
		}
	};

	/// <summary>
	///
	/// </summary>
	/// <param name="buffer"></param>
	/// <returns></returns>
	inline t_string toHex( const t_buffer & buffer, const t_char * hex )
	{
		t_string out( buffer.len * 2, 0 );

		for ( size_t i = 0; i < buffer.len; i++ ) {
			out[i << 1] = hex[( buffer.ptr[i] >> 4 ) & 0x0f];
			out[( i << 1 ) + 1] = hex[buffer.ptr[i] & 0x0f];
		}

		return out;
	}

	///
	inline t_string toHex( const t_buffer & buffer )
	{
		static t_char hex[] = AS_T( "0123456789ABCDEF" );
		return toHex( buffer, hex );
	}

	///
	inline t_string toHexLowerCase( const t_buffer & buffer )
	{
		static t_char hex[] = AS_T( "0123456789abcdef" );
		return toHex( buffer, hex );
	}

	/// <summary>
	///
	/// </summary>
	/// <param name="secret"></param>
	/// <param name="data"></param>
	/// <returns></returns>
	inline as::t_string toBase64( const t_buffer & buffer )
	{
		t_string result( 4 * ( ( buffer.len + 2 ) / 3 ), 0 );
		int len = EVP_EncodeBlock( reinterpret_cast<unsigned char *>(
									   const_cast<t_char *>( result.c_str() ) ),
			buffer.ptr,
			static_cast<int>( buffer.len ) );

		return result;
	}

	///
	inline std::vector<t_byte> hmacSha256(
		const as::t_stringview & secret, const as::t_string & data )
	{

		std::vector<t_byte> result( EVP_MAX_MD_SIZE );
		unsigned bufferLen;

		HMAC( EVP_sha256(),
			secret.data(),
			static_cast<int>( secret.length() ),
			reinterpret_cast<const unsigned char *>( data.c_str() ),
			static_cast<int>( data.length() ),
			result.data(),
			&bufferLen );

		result.resize( bufferLen );

		return result;
	}

	/// <summary>
	///
	/// </summary>
	inline std::vector<t_byte> hmacSha512(
		const as::t_stringview & secret, const as::t_string & data )
	{

		std::vector<t_byte> result( EVP_MAX_MD_SIZE );
		unsigned bufferLen;

		HMAC( EVP_sha512(),
			secret.data(),
			static_cast<int>( secret.length() ),
			reinterpret_cast<const unsigned char *>( data.c_str() ),
			static_cast<int>( data.length() ),
			result.data(),
			&bufferLen );

		result.resize( bufferLen );

		return result;
	}

	/// <summary>
	///
	/// </summary>
	inline as::t_string uuidString()
	{
		static thread_local boost::uuids::random_generator generator;
		return boost::uuids::to_string( generator() );
	}

} // namespace as


#endif
