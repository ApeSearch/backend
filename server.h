/*
 * server.h
 *
 * Header file for the server class.
 */
#ifndef _SERVER_H_
#define _SERVER_H_

// #include "fs_server.h"
// #include "file_system.h"
// #include "Client.h"
// #include "requestBody.h"
#include "socket.h"
#include <string>
#include <mutex> // For std::unique_lock
#include <shared_mutex> // For shared_mutex
// using std::shared_mutex;
using std::string;

/*
 * users: Unordered map to Client. Username serves as as a key to choose which
 *        password to use to decrypt request received as well as to refer to
 *        the username itself later on. More information on implementation of Client
 *        and its variables, refer to server.cpp and Client.h respectively.
*/
class Server {
public:
    Server(int port_number);
    Server(int port_number, char *file);
private:    
    void run_server();
    void handle_request(const int msg_sock);
    void receiveRequest(const int msg_sock);

    string formResponse();
    int listen_socket;
    Socket sock; // socket is last
};


#endif /* _SERVER_H_ */