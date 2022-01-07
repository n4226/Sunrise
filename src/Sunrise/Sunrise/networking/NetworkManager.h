#pragma once

#include "srpch.h"

#undef defer
#include <asio.hpp>

//namespace asio {
//	class io_context;
//	class mutable_buffer;
//	namespace ip {
//		namespace tcp {
//			class acceptor;
//		}
//		namespace udp {
//			class socket;
//			class endpoint;
//		}
//	}
//}

namespace sunrise {

	// from asio docs tutorial - asynch server tcp 
	class TCPConnection
	{
	public:

		//TCPConnection(NetworkManager* manager);

	//	asio::ip::tcp::socket& socket()
	//	{
	//		return socket_;
	//	}

	//	void connectionMade();


	//	//void sendMessage(Message& msg);
	//	//Message* recieveMessage();

	//private:

	//	NetworkManager* manager;

	//	//	void handle_write(const asio::error_code& /*error*/,
	//	//		size_t /*bytes_transferred*/);

		//asio::ip::tcp::socket socket_;

	//	void runConnectionLoop();
	};


	/// <summary>
	/// program to help debug sockets: https://www.hw-group.com/software/hercules-setup-utility
	/// 
	/// </summary>
	class SUNRISE_API NetworkManager
	{
	public:

		struct MessageHeader
		{//todo
			/*Message::MessageType type;
			size_t length;
			Message::Endpoint recipient;*/
		};

		enum class Type
		{
			client,
			server
		};

		struct CreateOptions {
			Type type;
			bool newThread = false;
			unsigned short port = 3050;
			size_t udpBufferSize = 10;
			bool deferServerStart;
		};

		NetworkManager(const CreateOptions& options, asio::io_context& context);
		~NetworkManager();
		/// <summary>
		/// can only call befoer server starts
		/// </summary>
		void registerUDPCalback(std::function<void(char*,size_t)> callback);

		template<typename T>
		void registerUDPMessageCalback(std::function<void(const T&)> callback);

		void startServer();

		void registerMessage();

	//	//void sendMessage(Ref<Message> msg);

	//	//std::function<void(Message*)>* messagerecievedHandler;

		//client only api
		void connect(const std::string& ip);

		
		void clientSend(void* data, size_t length);

		template<typename T>
		void clientSend(const T& message);

	protected:

		bool running = false;

		//std::unordered_map<size_t, > registeredMessages;

		friend TCPConnection;

		asio::io_context& context;

		void setupServer(asio::io_context& context);
		void setupClient(asio::io_context& context);

		CreateOptions options;


	//	// server vars
		asio::ip::tcp::acceptor* tcpAcceptor;
		asio::ip::udp::socket* udpSocket; 
		asio::ip::udp::endpoint remote_endpoint_;
		
		std::vector<char> udpBytes = {};
		asio::mutable_buffer udpBytesBuff;

		void tcp_acceptNext();
		void udp_acceptNext();

		//libguarded::plain_guarded<std::vector<TCPConnection*>> connections{};

		// client vars

		asio::ip::udp::endpoint* udpClient_EndPoint = 0;
		asio::ip::udp::socket* udpClient_socket = 0;

		//TODO: not guarded for multthreading
		std::function<void(char*, size_t)>* registeredCalbkack = nullptr;
		//libguarded::shared_guarded<std::function<void(char*, size_t)>*,std::mutex> registeredCalbkack = libguarded::shared_guarded<std::function<void(char*, size_t)>*, std::mutex>(nullptr);
		//libguarded::shared_guarded<bool> callbackRegestered = libguarded::shared_guarded<bool>(true);


	};

	template<typename T>
	inline void NetworkManager::clientSend(const T& message)
	{
		std::array<char, sizeof(T)> data;
		memcpy(data.data(), &message, data.size());
		clientSend(data.data(), data.size());
	}

	template<typename T>
	inline void NetworkManager::registerUDPMessageCalback(std::function<void(const T&)> callback) {
		registerUDPCalback([callback](char* data, size_t length) {
			if (length >= sizeof(T)) {
				T result;
				memcpy(&result, data, length);
				callback(result);
			}
		});
	}

}
