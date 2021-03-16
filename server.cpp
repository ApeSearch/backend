#include "server.h"
#include "json.hpp"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include "../libraries/AS/include/AS/string.h"
#include <time.h>
#include "libraries/AS/include/AS/algorithms.h"

using json = nlohmann::json;


//pthread_mutex_t resultsLock = PTHREAD_MUTEX_INITIALIZER;

std::vector<Result> possibleDocuments = {
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
Server::Server(int port_number) : sock( Socket(port_number) ), threadsPool( MAXCLIENTS * SERVERNODES, maxNumOfSubmits) {
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
    APESEARCH::string buf(Socket::MAX_SIZE, '\0');
    if (!sock.receive_request(&buf.front(), msg_sock, Socket::MAX_SIZE, 0))
        {
            std::cout << "Receive failed" << std::endl;
            return;
        }

    auto lineEnd = buf.find("\r\n");
    auto queryLine = APESEARCH::string(&buf.front(), lineEnd);
    std::cout << queryLine << std::endl;       
     
    std::vector<Result> resultDocuments = retrieveSortedDocuments();

    APESEARCH::string response = formResponse(resultDocuments);
    std::cout << response << std::endl;

    send(msg_sock, response.begin(), response.size(), MSG_NOSIGNAL);
} // end receiveRequest()

APESEARCH::string Server::formResponse(const std::vector<Result> &documents) 
    {
        std::ostringstream stream;
        stream << "HTTP/1.1 200 OK\r\n";
        stream << "Content-type: application/json\r\n";
        stream << "Access-Control-Allow-Origin: *\r\n";
        stream << "Connection: close\r\n";
        stream << "\r\n";
        
        stream << serializeResults(documents);

        return APESEARCH::string( stream.str().begin(), stream.str().end() );
    } // end formRequest()

APESEARCH::string Server::serializeResults(const std::vector<Result> &documents) {
    json response = json::array();
    
    size_t numOfDocs = APESEARCH::min( documents.size(), Server::maxTopDocs );
    for ( size_t i = 0; i < numOfDocs; ++i )
        response.push_back(json({{"url", documents[i].url}, {"snippet", documents[i].snippet}, {"rank", documents[i].rank}}));

    auto resp = response.dump();
    return APESEARCH::string( resp.begin(), resp.end() );
}

void sortResults(std::vector<Result> &documents){
    for(int i = 1; i < documents.size(); ++i){
        for(int index = i; index > 0 && documents[index].rank > documents[index - 1].rank; --index)
            APESEARCH::swap( documents[index], documents[index - 1]);
    }
}

void rankPage(Result &page) {
    page.rank = (float) rand() / RAND_MAX;
}

std::vector<Result> Server::getRandDocument(){
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
        futureObjs.emplace_back( threadsPool.submit( 
            [this]() -> std::vector<Result> { return this->getRandDocument(); } ) );
        //pthread_create(&(rpcPool[i]), NULL, &getRandDocument, (void*) &documents);

    for( std::future<std::vector<Result>>& futObj : futureObjs )
       {
        std::vector<Result> docsOfNode( futObj.get() );
        documents.insert( documents.end(), docsOfNode.begin(), docsOfNode.end() );
       } // end for
    sortResults(documents);
    return documents;
}
