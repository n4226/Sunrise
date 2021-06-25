#pragma once

#include "srpch.h"

namespace sunrise {

	//class NetworkManager
	//{
	//public:

	//	struct MessageHeader
	//	{//todo
	//		/*Message::MessageType type;
	//		size_t length;
	//		Message::Endpoint recipient;*/
	//	};

	//	enum class Type
	//	{
	//		client,
	//		server
	//	};

	//	NetworkManager(Type type, asio::io_context& context);



	//	//void sendMessage(Ref<Message> msg);

	//	//std::function<void(Message*)>* messagerecievedHandler;

	//protected:

	//	//void setupServer(asio::io_context& context);
	//	//void setupClient(asio::io_context& context);

	//	Type type;


	//	// server vars
	//	asio::ip::tcp::acceptor* acceptor;

	//	void acceptNext();

	//	libguarded::plain_guarded<std::vector<Connection*>> connections{};

	//};

}
