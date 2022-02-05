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

/// url.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__URL__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__URL__H


#include <string>

#include "core.hpp"


namespace as {

	class Url {
	protected:
		t_string m_uri;
		t_string m_scheme;
		t_string m_hostname;
		t_string m_path;
		uint16_t m_port;

	protected:
		Url() = default;

	public:
		Url( const t_stringview & uri )
			: m_port( 0 )
		{

			parse( *this, uri.data() );
		}

		static void parse( Url & out, const t_stringview & s )
		{
			out.m_uri = s;

			out.m_scheme = s.substr( 0, s.find( AS_T( ':' ) ) );

			auto tokenStart = s.find( AS_T( "//" ) );
			auto tokenEnd = s.find( AS_T( "/" ), tokenStart + 2 );

			out.m_hostname =
				s.substr( tokenStart + 2, tokenEnd - tokenStart - 2 );

			tokenStart = out.m_hostname.find( ':' );

			if ( t_string::npos != tokenStart ) {
				out.m_port = AS_STOI( out.m_hostname.substr( tokenStart + 1 ) );

				out.m_hostname.erase( tokenStart );
			}

			out.m_path = AS_T( '/' ) +
				t_string( t_string::npos == tokenEnd
						? AS_T( "" )
						: s.substr( tokenEnd + 1 ) );

			if ( out.m_port != 0 ) {
				return;
			}

			if ( AS_T( "https" ) == out.m_scheme ||
				AS_T( "wss" ) == out.m_scheme ) {

				out.m_port = 443;
			}
			else if ( AS_T( "http" ) == out.m_scheme ||
				AS_T( "ws" ) == out.m_scheme ) {

				out.m_port = 80;
			}
		}

		static Url parse( const t_stringview & s )
		{
			Url out;
			parse( out, s.data() );

			return out;
		}

		Url addPath( const t_stringview & s ) const
		{
			if ( s.empty() ) {
				return Url( *this );
			}

			if ( AS_T( '/' ) == s[0] ) {
				if ( AS_T( '/' ) == m_uri[m_uri.length() - 1] ) {
					return Url( m_uri + s.substr( 1 ).data() );
				}

				return Url( m_uri + s.data() );
			}

			if ( AS_T( '/' ) == m_uri[m_uri.length() - 1] ) {
				return Url( m_uri + s.data() );
			}

			return Url( m_uri + '/' + s.data() );
		}

		Url add( const t_stringview & s ) const
		{
			return Url( m_uri + s.data() );
		}

		constexpr const t_string & Uri() const
		{
			return m_uri;
		}

		constexpr const t_string & Scheme() const
		{
			return m_scheme;
		}

		constexpr const t_string & Hostname() const
		{
			return m_hostname;
		}

		constexpr const t_string & Path() const
		{
			return m_path;
		}

		constexpr uint16_t Port() const
		{
			return m_port;
		}
	};

}


#endif
