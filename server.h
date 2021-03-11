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
#include <pthread.h>
#include <mutex> // For std::unique_lock
#include <shared_mutex> // For shared_mutex
#include "libraries/AS/include/AS/pthread_pool.h"
// using std::shared_mutex;
using std::string;

struct Result {
    Result():url(""), snippet(""), rank(0){}
    Result(string _url, string _snippet, double _rank) : url(_url), snippet(_snippet), rank(_rank){}
    string url, snippet;
    double rank;
};

#define SERVERNODES 5
#define MAXCLIENTS 1
#define DOCSPERNODE 10

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

    string serializeResults(const std::vector<Result> &documents);
    string formResponse(const std::vector<Result> &documents);
    std::vector<Result> retrieveSortedDocuments();

    int listen_socket;
    Socket sock; // socket is last
    APESEARCH::PThreadPool<std::deque<APESEARCH::Func>> threadsPool;
    static constexpr size_t maxTopDocs = 10u;
};



#endif /* _SERVER_H_ */