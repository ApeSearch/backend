#include "server.h"
#include "json.hpp"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <string>
#include <time.h>

;


using std::string;
using json = nlohmann::json;




//pthread_mutex_t resultsLock = PTHREAD_MUTEX_INITIALIZER;

vector<Result> possibleDocuments = {
            Result("https://google.com", "A short description for google", 0),
            Result("https://amazon.com", "A short description for amazon", 0),
            Result("https://facebook.com", "A short description for facebook", 0),
            Result("https://bing.com", "A short description for bing", 0),
            Result("https://reddit.com", "A short description for reddit", 0),
            Result("https://nytimes.com", "A short description for nytimes", 0),
            Result("https://cnbc.com", "A short description for cnbc", 0),
            Result("https://instagram.com", "A short description for instagram", 0)
        };

/* Server::Server (custom constructor): creates Server object with a
                                        corresponding socket port number
 */
Server::Server(int port_number) : sock( Socket(port_number) ), threadsPool( MAXCLIENTS * SERVERNODES ) {
    srand(time(NULL));
    run_server();
}

/* this is leftover from 482 code, prolly ignore
 * The reason for this overload is to circumvent an issue with macos's vscode issue
 * where standard input cannot be read from. It inputs in the passwords into a filestream
 * And processes in the same manner.
*/
Server::Server(int port_number, char *file) : Server(port_number) {
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

    std::vector<Result> resultDocuments = retrieveSortedDocuments();

    std::cout << buf << std::endl;

    string response = formResponse(resultDocuments);

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
        response.push_back(json({{"url", documents[i].url}, {"snippet", documents[i].snippet}, {"rank", documents[i].rank}}));

    return response.dump();
}

void sortResults(std::vector<Result> &documents){
    for(int i = 1; i < documents.size(); ++i){
        int index = i;

        for(int index = i; index > 0 && documents[index].rank > documents[index - 1].rank; --index){
            Result temp = documents[index];
            documents[index] = documents[index - 1];
            documents[index - 1] = temp;
        }
    }
}

void rankPage(Result &page) {
    page.rank = (float) rand() / RAND_MAX;
}

std::vector<Result> getRandDocument(){
    std::vector<Result> documents;
    documents.reserve(DOCSPERNODE);

    for (int i = 0; i < DOCSPERNODE; ++i) {
        Result docToPush = possibleDocuments[rand() % possibleDocuments.size()];

        rankPage(docToPush);

        documents.push_back(docToPush);
    }
    return documents;
}


std::vector<Result> Server::retrieveSortedDocuments(){
    std::vector<Result> documents;
    std::vector< std::future<std::vector<Result>> > futureObjs;
    documents.reserve( SERVERNODES * DOCSPERNODE );
    futureObjs.reserve(SERVERNODES);

    for(size_t i = 0; i < SERVERNODES; ++i)
        futureObjs.emplace_back( threadsPool.submit(getRandDocument) );
        //pthread_create(&(rpcPool[i]), NULL, &getRandDocument, (void*) &documents);

    for(size_t i = 0; i < SERVERNODES; ++i)
       {
        std::vector<Result> docsOfNode( futureObjs[i].get() );
        documents.insert( documents.end(), docsOfNode.begin(), docsOfNode.end() );
       } // end for
    sortResults(documents);
    return documents;
}