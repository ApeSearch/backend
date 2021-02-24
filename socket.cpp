#include "socket.h"
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <algorithm>
using std::copy;
using std::find;

/*
 * EFFECTS: Effectively creates a socket handle, binding it to the port_number
 *          passed in (if zero, a random one) and starts listening
 *
 *  Notes: - AF_INET stands for the Internet domain.
 *         - SOCK_STREAM configures socket to utilize
 *         a stream socket with the TCP orptocol.
 *         - 0 is the protocol argument.
 *
 *         Binding Local Names:
 *         The bind function enables remote processes to be able to
 *         refer to a specific address, thus initiating the connection.
 *         More specifically, it allows a process to specific the local address
 *         of the socket: <local address, local port>
 *
 *         A connection requires both local and remote addresses.
 *         ntohs: network to host server
 *         htons: host to nework server
 *
*/
Socket::Socket(int port_number) : listen_socket(create_init_socket(port_number)) {}

int Socket::create_init_socket(int port_number) {
    struct sockaddr_in server;
    int return_code;

    // Clears everything in server to be the size of zero
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);

    // Create socket
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    handle_error(listen_socket == -1, "opening socket");

    // Set options allowing socket to reuse local addresses
    int option = 1;
    return_code = setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR , (char *) &option, sizeof(option));
    handle_error(return_code < 0, "setting socket options");

    // Bind assigns a port to the socket
    return_code = bind(listen_socket, (struct sockaddr *) &server, sizeof(server));
    handle_error(return_code == -1, "binding stream socket");

    // Get the socket's address (placed into the server buffer)
    socklen_t length = sizeof(server);
    return_code = getsockname(listen_socket, (struct sockaddr *) &server, &length);
    handle_error(return_code != 0, "getsockname failed");

    // Print port number and run server
    std::cout << "\n@@@ port " << ntohs(server.sin_port) << std::endl;
    listen(listen_socket, 30);

    return listen_socket;
} // create_init_socket()


// Server::handle_error: destroys proccess incase socket functions returns and error
void Socket::handle_error(bool condition, const char *msg) {
    if (condition) {
        perror(msg);
        exit(1);
    } // end if
} // end handle_error

bool handle_accept(const int msg_sock);
int Socket::get_connection_socket() {
    // Wait for the next request and check that it's valid
    int msg_sock = -1;
    do {
        msg_sock = accept(listen_socket, nullptr, 0);
    } while (handle_accept(msg_sock));

    return msg_sock;
} // end get_connection_socket()

bool handle_accept(const int msg_sock) {
    if (msg_sock == -1) {
        perror("accept");
        std::cerr << "Error: " << errno << std::endl;
        if((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
                errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH)) {
            return true;
        } // end if
        else {
            return true;
            //exit (EXIT_FAILURE);
        } // end else
    } // end if
    return false;
} // end handle_accept()

/*
 *
*/
bool Socket::receive_request(char * const buffer_ptr, const int msg_sock, const unsigned length_of_req, const unsigned bytes_recvd_so_far) {
    // Read request, this is really bad format but it works FOR NOW
    // will want to update to the same format used in our crawler to receive http requests from a client
    int bytes_recvd = recv(msg_sock, buffer_ptr, length_of_req - 1, 0);
    return bytes_recvd >= 0;

    // USE THIS PATTERN TO ENSURE ALL BYTES ARE RECEIVED
    // do {
    //     recvd_ = recv(msg_sock, buffer_ptr + bytes_recvd_so_far, length_of_req - totRecvd, 0);
    //     totRecvd += (unsigned)recvd_;
    // } while (recvd_ > 0); // end while
} // end receive_header()

/* leftover from 482 code, we can modify this to be the main response interface tho
 * REQUIRES: password to be non-sentinel
 * MODIFIES: Nothing
 * EFFECTS: Constructors the response message via the following layout:
 *         RESPONSE: <session> <sequence><NULL><data>
 *         Note: <data> is nothing in all scenerios except READBLOCK.
 *
 *         The function utilizes ostringstream to aid in constructing a string
 *         so that it may be encrypted using fs_encrpt and then send after the
 *          response header.
 *
 * Note To Self: Streams (especially stringstreams) tend to be very slow when
 * converting to a certain format (to and from strings). This is a tradeoff for
 * simplicity as it ensures that the format is (basically) correct.
 *
*/
void Socket::send_response(const string& password, const int msg_sock, const unsigned session,
                                const unsigned seq, const string& data) {
    
} //end send_response()