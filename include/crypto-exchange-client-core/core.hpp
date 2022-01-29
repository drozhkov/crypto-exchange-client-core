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


namespace as {

#define AS_T( a_t ) a_t
#define AS_STOI std::stoi
#define AS_CALL( a, ... )                                                      \
	if ( a ) {                                                                 \
		a( __VA_ARGS__ );                                                      \
	}


	using t_string = std::string;
	using t_char = char;
	using t_stringview = std::string_view;
	using t_byte = unsigned char;


	struct t_buffer {
		t_byte * ptr;
		size_t len;

	public:
		t_buffer( void * ptr, size_t len )
			: ptr( static_cast<t_byte *>( ptr ) )
			, len( len )
		{
		}
	};


	inline t_string toHex( t_buffer & buffer )
	{
		static t_char hex[] = AS_T( "0123456789abcdef" );

		t_string out( buffer.len * 2, 0 );

		for ( size_t i = 0; i < buffer.len; i++ ) {
			out[i << 1] = hex[( buffer.ptr[i] >> 4 ) & 0x0f];
			out[( i << 1 ) + 1] = hex[buffer.ptr[i] & 0x0f];
		}

		return out;
	}

}


#endif
