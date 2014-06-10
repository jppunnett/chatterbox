//	Simple Chatterbox client written in C++
//	Many of the functions are taken from the excellent Winsock FAQ at
//		http://tangentsoft.net/wskfaq/

#include <iostream>
#include <string>
#include <stdexcept>
#include <winsock2.h>
// #include <winsock.h>

using namespace std;


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

//// LookupAddress /////////////////////////////////////////////////////
// Given an address string, determine if it's a dotted-quad IP address
// or a domain address.  If the latter, ask DNS to resolve it.  In
// either case, return resolved IP address.  If we fail, we return
// INADDR_NONE.
u_long LookupAddress(const char* pcHost)
{
    u_long nRemoteAddr = inet_addr(pcHost);
    if (nRemoteAddr == INADDR_NONE) {
        // pcHost isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(pcHost);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        nRemoteAddr = *((u_long*)pHE->h_addr_list[0]);
    }

    return nRemoteAddr;
}


//// EstablishConnection ///////////////////////////////////////////////
// Connects to a given address, on a given port, both of which must be
// in network byte order.  Returns newly-connected socket if we succeed,
// or INVALID_SOCKET if we fail.
SOCKET EstablishConnection(u_long nRemoteAddr, u_short nPort)
{
    // Create a stream socket
    // SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd != INVALID_SOCKET) {
        SOCKADDR_IN sinRemote;
        sinRemote.sin_family = AF_INET;
        sinRemote.sin_addr.s_addr = nRemoteAddr;
        sinRemote.sin_port = nPort;
        if (connect(sd, (sockaddr*)&sinRemote, sizeof(sockaddr_in)) ==
                SOCKET_ERROR) {
            sd = INVALID_SOCKET;
        }
    }

    return sd;
}

//// ShutdownConnection ////////////////////////////////////////////////
// Gracefully shuts the connection sd down.  Returns true if we're
// successful, false otherwise.
bool ShutdownConnection(SOCKET sd)
{
	const int kBufferSize = 1024;
	// const int SD_SEND = 1;

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
    char acReadBuffer[kBufferSize];
    while (1) {
        int nNewBytes = recv(sd, acReadBuffer, kBufferSize, 0);
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


int main() {

	WASDataWrapper wsaData;

	//	Connect to a Chatterbox server
	string host = "localhost";
	u_short port = 20000;

    u_long remote_addr = LookupAddress(host.c_str());
    if (remote_addr == INADDR_NONE) {
        cerr << "Could not detremine remote host address." ;
        return 1;
    }

    cout << "remote_addr=" << remote_addr << '\n';
    in_addr address;
    memcpy(&address, &remote_addr, sizeof(u_long)); 
    cout << "Connecting to Chatterbox server at, " << inet_ntoa(address) << ":" << port << endl; 


	// Create a stream socket
	SOCKET sd = EstablishConnection(remote_addr, port);
	if(sd == INVALID_SOCKET) {
		cout << "Could not connect to Chatterbox server\n";
    	cerr << "WSAGetLastError() says:" << WSAGetLastError() << '\n';
		return 1;
	}

	cout << "Connected to Chatterbox server!";

	//	Let user enter an id
	//	auto userID = "";
	//	userID << cin;

	//	Send the user ID
	//	socket.send(userID);

	//	Create a sender thread and listener thread
	//	Start listener thread
	//	Start sender thread

	//	Wait for threads to exit

	// Shut connection down
	cout << "Disconnectiong from Chatterbox server.\n";
	if (ShutdownConnection(sd)) {
	    cout << "Disconnected successfully.\n" << endl;
	}
	else {
	    cout << "Problem disconnecting.\n";
	}

	cout << "Exiting Chatterbox client." << endl;

	return 0;
}
