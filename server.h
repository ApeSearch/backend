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
#include "libraries/AS/include/AS/string.h"
#include <string>
#include <pthread.h>
#include "libraries/AS/include/AS/mutex.h" // For std::unique_lock
#include <shared_mutex> // For shared_mutex
#include "libraries/AS/include/AS/pthread_pool.h"
#include "libraries/AS/include/AS/circular_buffer.h"
// using std::shared_mutex;

struct Result {
    Result():url(""), snippet(""), rank(0){}
    Result(std::string _url, std::string _snippet, double _rank) : url(_url), snippet(_snippet), rank(_rank){}
    std::string url, snippet;
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

    APESEARCH::string serializeResults(const std::vector<Result> &documents);
    APESEARCH::string formResponse(const std::vector<Result> &documents);
    std::vector<Result> retrieveSortedDocuments();
    std::vector<Result> getRandDocument();

    int listen_socket;
    backend::Socket sock; // socket is last
    static constexpr size_t maxNumOfSubmits = MAXCLIENTS * DOCSPERNODE;
    APESEARCH::PThreadPool<APESEARCH::circular_buffer
    <APESEARCH::Func, APESEARCH::DEFAULT::defaultBuffer<APESEARCH::Func, maxNumOfSubmits> >> threadsPool;
    static constexpr size_t maxTopDocs = 10u;
};



#endif /* _SERVER_H_ */