//	Simple Chatterbox client written in C++
//	Depends on boost 1.55 and MinGW.
//	
//	To build on Windows 64-bit systems, we used the excellent MinGW Distro
//	from nuwen.net. Build command is:
//		g++ -std=c++0x -Wall -o cbclient cbclient.cpp -lboost_thread -lboost_system -lws2_32
//
//	When building on Windows XP SP3, define NTDDI_VERSION=0x05010300.
//	The build command becomes:
//		g++ -std=c++0x -Wall -DNTDDI_VERSION=0x05010300 -o cbclient cbclient.cpp -lboost_thread -lboost_system -lws2_32
//
//	If you are not using the MinGW Distro from nuwen.net, you'll need to
//	build the Boost libraries yourself. This could possibly be a difficult 
//	but worthy task. Be strong.
//	
//	Caution when building Boost 1.55 under MinGW on XP. You SHOULD convert the build.bat
//	from Unix line endings to DOS line endings and you MUST ensure that the TMP and
//	TEMP user variables do not point to directory names that have spaces in them.
//
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

using std::ostream;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

using boost::asio::ip::tcp;


bool get_cmdline_args(int argc, char* argv[], string &host, string &port);
string socket_gets(tcp::socket& socket);
void socket_puts(tcp::socket& socket, const string& msg);
void do_listen(tcp::socket& socket);
void do_talk(tcp::socket& socket);
string chomp(const string& s);
void put_to_stream(ostream& o, const string& s);

//	Ugh, I know, global variables, but this will change. We're experimenting
//		for now.
bool done_chatting = false;
boost::mutex done_chatting_mtx;
boost::mutex output_mtx;
bool get_done_chatting();
void set_done_chatting();

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
		socket_puts(socket, user_id+'\n');

		boost::thread listen_thread(do_listen, boost::ref(socket));
		boost::thread talk_thread(do_talk, boost::ref(socket));

		listen_thread.join();
		talk_thread.join();
	}
	catch (std::exception& e) {
		cerr << e.what() << endl;
	}

	return 0;
}

void do_listen(tcp::socket& socket) {
	// cout << "In do_listen()" << endl;
	
	while(!get_done_chatting()) {

		string msg_received = socket_gets(socket);

		if(!msg_received.empty()) {
			// cout << '[' << msg_received << ']';
			cout << msg_received << endl;
			if(msg_received == "bye")
				set_done_chatting();
		}

	}

	// cout << "Leaving do_listen()" << endl;
}

void do_talk(tcp::socket& socket) {
	// cout << "In do_talk()" << endl;

	while(!get_done_chatting()) {

		string msg_to_send;
		getline(cin, msg_to_send);

		if(!msg_to_send.empty()) {
			socket_puts(socket, msg_to_send+'\n');
			if(msg_to_send == "bye") {
				set_done_chatting();
			}
		}
	}

	// cout << "Leaving do_talk()" << endl;
}


void put_to_stream(ostream& o, const string& s) {
	boost::lock_guard<boost::mutex> guard(output_mtx);
	o << s;
}


string chomp(const string& s) {
	
	const std::string whitespaces (" \t\f\v\n\r");
	string chomped = s;

	std::size_t found = chomped.find_last_not_of(whitespaces);
	if (found != std::string::npos)
		chomped.erase(found+1);
	else
		chomped.clear();

	return chomped;
}

bool get_done_chatting() {
	boost::lock_guard<boost::mutex> guard(done_chatting_mtx);
	return done_chatting;
}

void set_done_chatting() {
	boost::lock_guard<boost::mutex> guard(done_chatting_mtx);
	done_chatting = true;
}


string socket_gets(tcp::socket& socket) {

	std::array<char, 512> buf; 
	boost::system::error_code error;
	size_t len = socket.read_some(boost::asio::buffer(buf), error);
	if(error)
		throw boost::system::system_error(error);

	// cout << "socket_gets(), len=" << len << '\n';

	std::stringstream ss;
	ss.write(buf.data(), len);
	return chomp(ss.str());
}


void socket_puts(tcp::socket& socket, const string& msg) {
	boost::asio::write(socket, boost::asio::buffer(msg));
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
