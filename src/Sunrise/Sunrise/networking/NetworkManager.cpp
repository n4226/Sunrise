#include "srpch.h"
#include "NetworkManager.h"



using asio::ip::tcp;
using namespace asio::ip;

namespace sunrise {

	NetworkManager::NetworkManager(const CreateOptions& options, asio::io_context& context)
		: options(options), context(context)
	{
		switch (options.type)
		{
		case Type::server:
			if (!options.deferServerStart) {
				running = true;
				setupServer(context);
			}
		case Type::client:
			setupClient(context);
			break;
		}
	}

	NetworkManager::~NetworkManager()
	{
		switch(options.type)
		{
		case Type::server:
			delete udpSocket;
		case Type::client:
			
			break;
		}
	}

	void NetworkManager::registerUDPCalback(std::function<void(char*, size_t)> callback)
	{
		registeredCalbkack = new std::function(callback);
		//auto handle = registeredCalbkack.lock();
		//(*handle) = new std::function(callback);
	}


	void NetworkManager::setupServer(asio::io_context& context)
	{
		// tcp
	/*	tcpAcceptor = new tcp::acceptor(context, tcp::endpoint(tcp::v4(), options.port));

		if (options.newThread) {
			auto t = std::thread([this] {
				this->tcp_acceptNext();
				});
			t.detach();
		}
		else {
			this->tcp_acceptNext();
		}*/

		// udp
		udpSocket = new udp::socket(context, udp::endpoint(udp::v4(), options.port));

		udpBytes.resize(options.udpBufferSize);
		udpBytesBuff = asio::mutable_buffer(udpBytes.data(),udpBytes.size());

		udp_acceptNext();

	}

	void NetworkManager::setupClient(asio::io_context& context)
	{

	}


	void NetworkManager::startServer()
	{
		SR_ASSERT(options.type == Type::server && !running);

		running = true;
		setupServer(context);
	}

	void NetworkManager::connect(const std::string& ip)
	{
		udpClient_EndPoint = new udp::endpoint(asio::ip::address::from_string(ip),options.port);

		udpClient_socket = new udp::socket(context);
		udpClient_socket->open(udp::v4());
	}

	void NetworkManager::clientSend(void* data, size_t length)
	{
		auto buff = asio::buffer((const void*)data, length);
		udpClient_socket->send_to(buff,*udpClient_EndPoint);
	}

	void NetworkManager::tcp_acceptNext()
	{
		// tcp stuff
		//auto new_connection = new TCPConnection();

		//tcpAcceptor->async_accept(new_connection->socket(), [this, new_connection](asio::error_code ec) {
		//	if (!ec) {
		//		SR_CORE_TRACE("New client Connection");

		//		// ask client to identify themself
		//		auto t = std::thread([new_connection] {
		//			//new_connection->connectionMade();
		//		});
		//		t.detach();
		//		// add connection to list of cons
		//		auto handle = connections.lock();
		//		handle->push_back(new_connection);

		//	}
		//	else {
		//		delete new_connection;
		//	}
		//	//accept next client
		//	acceptNext();
		//});



	}

	void NetworkManager::udp_acceptNext()
	{
		//auto size = udpSocket->receive_from(udpBytesBuff, remote_endpoint_);


		udpSocket->async_receive_from(udpBytesBuff, remote_endpoint_, [this](asio::error_code err, size_t byteCount) {
			//SR_CORE_INFO("data Recieved with size {}, message:  {}", byteCount, std::string(udpBytes.begin(),udpBytes.begin() + byteCount));

			//auto handle = registeredCalbkack.lock();
			if (registeredCalbkack) {
				(*registeredCalbkack)(udpBytes.data(), udpBytes.size());
			}

			udp_acceptNext();
		});
	}

	//TCPConnection::TCPConnection(NetworkManager* manager)
	//	: socket_(manager->context)//, manager(manager)
	//{

	//}

	/*
	void Connection::connectionMade()
	{
	}

	void Connection::runConnectionLoop()
	{
	}*/

}