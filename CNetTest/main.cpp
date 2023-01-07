
#include<iostream>
#include<thread>
#include<string>
#include<map>

#include<CNet.h>
#include<CNetServer.h>

using namespace cnet;

#define random(x)  static_cast<float>(rand())/static_cast<float>(x)

class HTTPServer : public CNetServer
{
public:
	HTTPServer(CNetProps props) : CNetServer(props)
	{
	}

protected:
	std::map<int, std::string> tmp;

	bool on_client_new(std::shared_ptr<CNetClient> client) override
	{
		std::cout << "Client: " << client->get_id() << " has join" << std::endl;

		return true;
	}

	bool on_client_recieve(std::shared_ptr<CNetClient> client, char* buffer, size_t byteread) override
	{

		for (size_t i = 0; i < byteread; i++)
		{
			tmp[client->get_id()] += buffer[i];
		}
		std::cout << client->get_id() << " " << byteread << " bytes" << std::endl;

		if (byteread < this->m_props.packet_size)
		{
			std::cout << "Recieve (" << tmp[client->get_id()].size() << " bytes) " << std::endl << tmp[client->get_id()] << std::endl;
			tmp[client->get_id()] = "";


			client->send(R"(HTTP/1.1 200 OK
Content-Type: text/html

<html><body><h1>Hello, World!</h1></body></html>
)");
			return false;
		}

		return true;
	}

	void on_client_quit(std::shared_ptr<CNetClient> client) override
	{
		std::cout << " Client: " << client->get_id() << " has quit" << std::endl;
	}
};

#pragma pack(push, 1)
typedef struct Vec2 {
	float x, y;
}Vec2;
#pragma pack(pop)

int http()
{
	char* host = (char*)"127.0.0.1";//cnet_inaddr_any();
	short port = 8080;

	const char* protocol = "TCP";
	auto props = cnet_create_props(host, port);

	HTTPServer* server = new HTTPServer(props);
	if (server->start() == CNetStatus::CNET_ERROR)
	{
		server->terminate();
		return 1;
	}
	std::cout << "internal: " << host << ":" << port << std::endl;

	while (server->is_alive())
	{
		std::cout << server->count_clients() << std::endl;
		std::cout << ">";
		std::string line;
		std::getline(std::cin, line);
		if (line == "exit")
		{
			break;
		}
	}

	server->terminate();
	return 0;
}

int buff()
{
	CNetProps props = cnet_create_props((char*)"127.0.0.1", 8080);
	CNetServer* srv = new CNetServer(props);
	srv->start();

	CNetClient* cl1 = new CNetClient(props);
	CNetClient* cl2 = new CNetClient(props);
	cl1->start();
	cl2->start();

	bool spam = false;


	

	std::thread([&]() {
		while (cl2->is_alive() && cl1->is_alive())
		{
			if (spam)
			{
				Vec2 inVec = { 14.876f, 0.2398623f  };
				cl1->send(CNetBuffer(&inVec));
			}
		
			CNetBuffer buffer;
			if (cl2->recieve(buffer) != CNetStatus::CNET_ERROR)
			{
				Vec2* outVec = buffer;
				std::cout << (!spam ? "[TIME OUT] " : "") << outVec->x << " " << outVec->y << std::endl;
			}
			buffer.clear();
			
		}
	}).detach();

	std::cin.get();
	spam = true;
	std::cin.get();
	spam = false;
	std::cin.get();

	srv->terminate();
	
	cl1->terminate();	
	cl2->terminate();

	return 0;
}

int main(int argc, const char** argv)
{
	return buff();
}