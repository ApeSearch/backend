#include "server.h"
// #include "Client.h"
// #include "requestBody.h"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>
#include <limits>
#include "assert.h"
#include <thread>
#include <unistd.h>
#include <string.h>


/* Server::Server (custom constructor): creates Server object with a
                                        corresponding socket port number
 */
Server::Server(int port_number) : sock( Socket(port_number) ) {
    run_server();
}

/* this is leftover from 482 code, prolly ignore
 * The reason for this overload is to circumvent an issue with macos's vscode issue
 * where standard input cannot be read from. It inputs in the passwords into a filestream
 * And processes in the same manner.
*/
Server::Server(int port_number, char *file) : sock( Socket(port_number) ) {
    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cerr << file << " isn't open" << std::endl;
        exit(1);
    }
    run_server();
}

void Server::run_server() {
    // Start accepting connections.
    while (1) {
        int msg_sock = sock.get_connection_socket();
        // Continue to handle_request
        // will need to update to use pthreads
        std::thread req(&Server::handle_request, this, msg_sock);
        req.detach();
    } // end while
    close(listen_socket);
} // end run_server()


/*
 *  Server::handle_request: parse and respond to messages(buffer) coming
 *                         into socket at port_number accordingly
 */
void Server::handle_request(const int msg_sock) {
    receiveRequest(msg_sock);
    close(msg_sock);
} // end handle_error

void Server::receiveRequest(const int msg_sock) {
    string buf(Socket::MAX_SIZE, '\0');
    if (!sock.receive_request(&buf.front(), msg_sock, Socket::MAX_SIZE, 0))
        {
            std::cout << "Receive failed" << std::endl;
            return;
        }

    std::cout << buf << std::endl;

    auto response = formResponse();  
    send(msg_sock, response.c_str(), response.length(), MSG_NOSIGNAL);
} // end receiveRequest()

string Server::formResponse() 
    {
    std::ostringstream stream;
    stream << "HTTP/1.1 200 OK\r\n";
    stream << "Content-type: text/html\r\n";
    stream << "Access-Control-Allow-Origin: *\r\n";
    stream << "Connection: close\r\n";
    stream << "\r\n"; 

    std::ifstream inputFile("index.html");

    stream << inputFile.rdbuf();
    return stream.str();
    } // end formRequest()