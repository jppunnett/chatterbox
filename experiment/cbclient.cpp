//	Simple Chatterbox client written in C++
//	Many of the functions are taken from the excellent Winsock FAQ at
//		http://tangentsoft.net/wskfaq/
//	Build using:
//		g++ -std=c++0x -Wall -o cbclient cbclient.cpp -lws2_32

#define _WIN32_WINNT 0x501

#include <iostream>
#include <string>
#include <stdexcept>
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

//  Default buffer size for reading socket data.
const int DEFAULT_BUF_SIZE = 1024;

class WASDataWrapper {
	WSADATA wsaData_;
public:	
	WASDataWrapper() {
		if(WSAStartup(MAKEWORD(2,2), &wsaData_) != 0) {
			throw runtime_error("WSAStartup failed");
		}
	}

	~WASDataWrapper() {
		WSACleanup();
	}

};


//// ShutdownConnection ////////////////////////////////////////////////
// Gracefully shuts the connection sd down.  Returns true if we're
// successful, false otherwise.
bool ShutdownConnection(SOCKET sd)
{
    // Disallow any further data sends.  This will tell the other side
    // that we want to go away now.  If we skip this step, we don't
    // shut the connection down nicely.
    if (shutdown(sd, SD_SEND) == SOCKET_ERROR) {
        return false;
    }

    // Receive any extra data still sitting on the socket.  After all
    // data is received, this call will block until the remote host
    // acknowledges the TCP control packet sent by the shutdown above.
    // Then we'll get a 0 back from recv, signalling that the remote
    // host has closed its side of the connection.
    char acReadBuffer[DEFAULT_BUF_SIZE];
    while (1) {
        int nNewBytes = recv(sd, acReadBuffer, DEFAULT_BUF_SIZE, 0);
        if (nNewBytes == SOCKET_ERROR) {
            return false;
        }
        else if (nNewBytes != 0) {
            cerr << endl << "FYI, received " << nNewBytes <<
                    " unexpected bytes during shutdown." << endl;
        }
        else {
            // Okay, we're done!
            break;
        }
    }

    // Close the socket.
    if (closesocket(sd) == SOCKET_ERROR) {
        return false;
    }

    return true;
}


void dump_addrinfo(addrinfo *result) {

    for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        cout << "ptr->ai_flags = " << ptr->ai_flags << '\n';
        cout << "ptr->ai_family = " << ptr->ai_family << '\n';
        cout << "ptr->ai_socktype = " << ptr->ai_socktype << '\n';
        cout << "ptr->ai_protocol = " << ptr->ai_protocol << '\n';
        cout << "ptr->ai_addrlen = " << ptr->ai_addrlen << '\n';
        cout << "ptr->ai_canonname = " << ptr->ai_canonname << '\n';
    }
}

SOCKET connect_to_CB_server(const char* hostname, const char* port) {

    //  Try to resolve server address
    // ADDRINFO Hints, *AddrInfo, *AI;
    addrinfo hints;
    memset(&hints, 0, sizeof (addrinfo));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo *result = NULL;
    int ret_val = getaddrinfo(hostname, port, &hints, &result);
    if (ret_val != 0) {
        cerr << "Cannot resolve address " << hostname << "and port " << port << ", error " << ret_val << '\n';
        return INVALID_SOCKET;
    }

    // dump_addrinfo(result);
    SOCKET s = INVALID_SOCKET;
    for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if(s == INVALID_SOCKET) {
            cerr << "socket() failed. WSAGetLastError() says:" << WSAGetLastError() << '\n';
        }
        else {
            //  Try to connect
            if(connect(s, ptr->ai_addr, ptr->ai_addrlen) != SOCKET_ERROR) {
                break;
            }
            else {
                cerr << "connect() failed. WSAGetLastError() says:" << WSAGetLastError() << '\n';
                closesocket(s);
                s = INVALID_SOCKET;
            }
        }
    }

    freeaddrinfo(result);

    return s;
}

//  Reads all data from a socket up to and excluding newline character
string socket_gets(SOCKET s) {

    char buffer[DEFAULT_BUF_SIZE];
    int bytes_read = recv(s, buffer, DEFAULT_BUF_SIZE, 0);
    if (bytes_read == SOCKET_ERROR) {
        throw runtime_error("recv() failed.");
    }
    cout << "bytes_read = " << bytes_read << '\n';

    return string(buffer);
}

int socket_puts(SOCKET s, const string& to_send) {

    int bytes_sent = send(s, to_send.c_str(), to_send.length(), 0);
    if (bytes_sent == SOCKET_ERROR) {
        throw runtime_error("send() failed.");
    }
    cout << "bytes_sent = " << bytes_sent << '\n';

    return bytes_sent;
}

void send_messages(SOCKET s) {

}

void listen_for_messages(SOCKET s) {

}

int main() {

	WASDataWrapper wsaData;

	//	Connect to a Chatterbox server
	string host = "localhost";
	string port = "20000";

    cout << "Attempting to connect to Chatterbox server at, " << host << ":" << port << '\n'; 

    SOCKET sd = connect_to_CB_server(host.c_str(), port.c_str());

	if(sd == INVALID_SOCKET) {
		cout << "Could not connect to Chatterbox server\n";
    	cerr << "WSAGetLastError() says:" << WSAGetLastError() << '\n';
		return 1;
	}

	cout << "Connected to Chatterbox server!\n";

    //  Display the Chatterbox welcome message.
    string msg_recvd = socket_gets(sd);
    cout << msg_recvd << '\n';

	//	Let user enter an id
    string user_id;
    getline(cin, user_id);

	//	Send the user ID
    socket_puts(sd, user_id);

	//	Create a sender thread and listener thread
    std::thread sender_th {send_messages, sd};
    std::thread receiver_th {listen_for_messages, sd};

    //  Wait for threads to exit
    sender_th.join();
    receiver_th.join();

	// Shut connection down
	cout << "Disconnecting from Chatterbox server.\n";
	if (ShutdownConnection(sd)) {
	    cout << "Disconnected successfully.\n";
	}
	else {
	    cout << "Problem disconnecting.\n";
	}

	cout << "Exiting Chatterbox client.\n";

	return 0;
}
