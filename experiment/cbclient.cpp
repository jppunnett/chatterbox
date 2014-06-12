//	Simple Chatterbox client written in C++
//	Uses boost 1.55.
//	To build on Windows, we used the excellent MinGW Distro from nuwen.net
//	http://nuwen.net/mingw.html:
//		g++ -std=c++0x -Wall -o cbclient cbclient_boost.cpp -lboost_thread -lboost_system -lws2_32
//	Usage:
//		cbclient <chatterbox-host> [<chatterbox-port>]
//		Default chatterbox-port is 20000
//
#include <iostream>
#include <string>
#include <sstream>
#include <array>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

using boost::asio::ip::tcp;


bool get_cmdline_args(int argc, char* argv[], string &host, string &port);
string socket_gets(tcp::socket& socket);
void socket_puts(tcp::socket& socket, const string& msg);


int main(int argc, char* argv[]) {

	string host, port;

	if(get_cmdline_args(argc, argv, host, port) == false) {
		cout << "Usage: cbclient <chatterbox-host> [<chatterbox-port>]";
		return 1;
	}

	try {

		cout << "Connecting to chatterbox server at " << host << ':' << port << endl;

		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(host, port);
		auto endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		//	Read and display the chatterbox welcome message.
		cout << socket_gets(socket) << endl;

		//	Let user enter an id
		string user_id;
		getline(cin, user_id);

		//	Send the user ID
		socket_puts(socket, user_id);

		//	Say goodbye
		socket_puts(socket, "bye");
	}
	catch (std::exception& e) {
		cerr << e.what() << endl;
	}

	//boost::thread t;
	return 0;
}


string socket_gets(tcp::socket& socket) {

	std::array<char, 512> buf; 
	boost::system::error_code error;
	
	size_t len = socket.read_some(boost::asio::buffer(buf), error);
	if(error)
		throw boost::system::system_error(error);

	std::stringstream ss;
	ss.write(buf.data(), len);
	return ss.str();
}


void socket_puts(tcp::socket& socket, const string& msg) {
	// boost::system::error_code ignored_error;
	boost::asio::write(socket, boost::asio::buffer(msg) /*, ignored_error*/);
}


bool get_cmdline_args(int argc, char* argv[], string &host, string &port) {

	if(argc < 2 || argc > 3) {
		return false;
	}

	if(argc > 1) {
		host = argv[1];
	}

	port = "20000";
	if(argc > 2) {
		port = argv[2];
	}

	return true;
}
