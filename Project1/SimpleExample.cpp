#include <iostream>
#include <chrono>

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using namespace std;

std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()), 
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				cout << "\nRead " << length << " bytes\n\n";
				for (int i = 0; i < length; i++)
				{
					cout << vBuffer[i];
				}
				GrabSomeData(socket);
			}
		}
	);
}

int main()
{
	asio::error_code ec;

	// Create a "context"
	asio::io_context context;

	// Give some fake tasks to asio so the context doesn't finish
	asio::io_context::work idleWork(context);

	// Start the context
	std::thread thrContext = std::thread([&]() { context.run(); });

	// Get the address of somewhere we wish to connect to
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	// Create a socket, the context will deliver the implementation
	asio::ip::tcp::socket socket(context);

	// Tell socket to try and connect
	socket.connect(endpoint, ec);

	if (!ec)
	{
		cout << "Connected!" << endl;
	}
	else
	{
		cout << "Failed to connect to address: \n" << ec.message() << endl;
	}

	if (socket.is_open())
	{
		GrabSomeData(socket);

		std::string sRequest =
			"Get /index.html HTTP/1.1\r\n"
			"Host: david-barr.co.uk\r\n"
			"Connection: close \r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

		//socket.wait(socket.wait_read);
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();
	}

	return 0;
}