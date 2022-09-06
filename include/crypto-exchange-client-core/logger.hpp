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

/// logger.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__LOGGER__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__LOGGER__H


#include <iostream>


namespace as {

#if 1
#define AS_LOG_TRACE( m )
#define AS_LOG_DEBUG( m )
#else
#define AS_LOG_DEBUG( a_m )                                                    \
	std::cout << __FILE__ << AS_T( ", " ) << __func__ << AS_T( ':' )           \
			  << __LINE__ << AS_T( ":: " ) << a_m

#define AS_LOG_TRACE( a_m )                                                    \
	std::cout << __FILE__ << AS_T( ", " ) << __func__ << AS_T( ':' )           \
			  << __LINE__ << AS_T( ":: " ) << a_m
#endif

#define AS_LOG_INFO( a_m )                                                     \
	std::cout << __FILE__ << AS_T( ", " ) << __func__ << AS_T( ':' )           \
			  << __LINE__ << ":: " << a_m

#define AS_LOG_ERROR( a_m )                                                    \
	std::cerr << __FILE__ << AS_T( ", " ) << __func__ << AS_T( ':' )           \
			  << __LINE__ << AS_T( ":: " ) << a_m

#define AS_LOG_TRACE_LINE( a_m ) AS_LOG_TRACE( a_m << std::endl )

#define AS_LOG_DEBUG_LINE( a_m ) AS_LOG_DEBUG( a_m << std::endl )

#define AS_LOG_INFO_LINE( a_m ) AS_LOG_INFO( a_m << std::endl )

#define AS_LOG_ERROR_LINE( a_m ) AS_LOG_ERROR( a_m << std::endl )

} // namespace as


#endif
