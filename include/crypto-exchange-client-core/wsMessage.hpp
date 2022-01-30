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

/// wsMessage.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_CORE__WS_MESSAGE__H
#define __CRYPTO_EXCHANGE_CLIENT_CORE__WS_MESSAGE__H


#include <memory>


namespace as::cryptox {

	using t_ws_message_type_id = int;

	class WsMessage {
	protected:
		static std::shared_ptr<WsMessage> s_unknown;

		t_ws_message_type_id m_typeId;

	public:
		static const as::cryptox::t_ws_message_type_id TypeIdUnknown = 1;

	public:
		WsMessage( t_ws_message_type_id typeId )
			: m_typeId( typeId )
		{
		}

		virtual ~WsMessage() = default;

		t_ws_message_type_id TypeId() const
		{
			return m_typeId;
		}
	};

}


#endif
