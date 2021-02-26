#include "server.h"
#include "json.hpp"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <string>

using std::string;
using json = nlohmann::json;

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
    std::string buf(Socket::MAX_SIZE, '\0');
    if (!sock.receive_request(&buf.front(), msg_sock, Socket::MAX_SIZE, 0))
        {
            std::cout << "Receive failed" << std::endl;
            return;
        }

    std::cout << buf << std::endl;

    std::vector<Result> documents = {
        Result("https://google.com", "A short description for google"),
        Result("https://amazon.com", "A short description for amazon"),
        Result("https://facebook.com", "A short description for facebook")
    };

    string response = formResponse(documents); 

    send(msg_sock, response.c_str(), response.length(), MSG_NOSIGNAL);
} // end receiveRequest()

string Server::formResponse(std::vector<Result> &documents) 
    {
        std::ostringstream stream;
        stream << "HTTP/1.1 200 OK\r\n";
        stream << "Content-type: application/json\r\n";
        stream << "Access-Control-Allow-Origin: *\r\n";
        stream << "Connection: close\r\n";
        stream << "\r\n";

        stream << serializeResults(documents);

        return stream.str();
    } // end formRequest()

string Server::serializeResults(std::vector<Result> &documents) {
    json response = json::array();

    for ( int i = 0; i < documents.size(); ++i )
        response.push_back(json({{"url", documents[i].url}, {"snippet", documents[i].snippet}}));

    return response.dump();
}