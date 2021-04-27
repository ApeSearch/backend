#include "server.h"
#include "json.hpp"
#include <iostream>
#include <utility>
#include <fstream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include "libraries/AS/include/AS/string.h"
#include <string>
#include <time.h>
#include "libraries/AS/include/AS/algorithms.h"
#include <arpa/inet.h>

using json = nlohmann::json;



/* Server::Server (custom constructor): creates Server object with a
                                        corresponding socket port number
 */
Server::Server(int port_number) : sock( backend::Socket(port_number) ), threadsPool( MAXCLIENTS * SERVERNODES, maxNumOfSubmits) {
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
    APESEARCH::string buf(backend::Socket::MAX_SIZE, '\0');
    if (!sock.receive_request(&buf.front(), msg_sock, backend::Socket::MAX_SIZE, 0))
        {
            std::cout << "Receive failed" << std::endl;
            return;
        }

    size_t start = 7;
    size_t end = buf.find("HTTP/1.1");
    APESEARCH::string queryLine = APESEARCH::string(&buf.front() + start, &buf.front() + end);
    //std::replace( queryLine.begin(), queryLine.end(), "%20", " "); //TODO

    std::vector<Result> resultDocuments = retrieveSortedDocuments(queryLine);

    APESEARCH::string response = formResponse(resultDocuments);

    std::cout << "-------START SENDING-------\n";
    send(msg_sock, response.begin(), response.size(), 0);
    
    std::cout << "-------FINISHED SENDING-------\n";
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
        auto str( stream.str() );

        return APESEARCH::string( str.begin(), str.end() );
    } // end formRequest()

APESEARCH::string Server::serializeResults(const std::vector<Result> &documents) {
    json response = json::array();
    
    size_t numOfDocs = APESEARCH::min( documents.size(), Server::maxTopDocs );
    for ( size_t i = 0; i < numOfDocs; ++i )
        response.push_back(json({{"url", std::string( documents[i].url ) }, {"rank", documents[i].rank}}));

    auto resp = response.dump();
    return APESEARCH::string( resp.begin(), resp.end() ); // big brain strats bois
}

void sortResults(std::vector<Result> &documents){
    for(int i = 1; i < documents.size(); ++i){
        for(int index = i; index > 0 && documents[index].rank > documents[index - 1].rank; --index)
            APESEARCH::swap( documents[index], documents[index - 1] );
    }
}


std::vector<Result> callNode(int node, APESEARCH::string &query )
{

    static std::vector<char *> ips = { "35.230.41.55", "34.71.229.2", "34.75.57.124", "35.245.134.242", "35.194.60.3", "35.232.126.246", "199.223.236.235", "35.194.73.46", "35.231.170.57", "34.86.240.51", "34.73.221.32", "34.86.225.197" };

    std::vector<Result> res;

    //Create addr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;    // select internet protocol 
    addr.sin_port = htons(6666);         // set the port # 
    addr.sin_addr.s_addr = inet_addr(ips[node]); // set the addr

    //Create socket
    int sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( sock < 0 )
        return res;
    
    //Set timeout
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec  = 0;

    if(setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv ) == -1 )
    {
        close( sock );
        return res;
    }
    

    if(setsockopt( sock, SOL_SOCKET, SO_SNDTIMEO, (const char*) &tv, sizeof tv ) == -1 )
    {
        close( sock );
        return res;
    }


    //Connect
    if( connect( sock, ( struct sockaddr * ) &addr, sizeof( addr ) ) < 0)
    {
        std::cout << "Could not connect to node: " << node << '\n';
        close( sock );
        return res;
    }

    //Send query to node
    if( send( sock, &query.front(), query.size() + 1, 0 ) < 0 )
    {
        close( sock );
        return res;
    }

    //recv and parse results into vector.
    int max_size = 65536;
    char buffer[max_size];

    int total_recieved = 0;
    int recieved = 0;

    while( ( recieved = recv( sock, buffer, max_size - total_recieved, 0 ) ) > 0 )
    {
        total_recieved += recieved;

        if( total_recieved == max_size)
        {
            close( sock );
            return res;
        }

        if( buffer[total_recieved - 1] == '\t')
            break;
    }

    Result r;
    char *ptr = buffer;
    char *end = ptr + total_recieved;
    char *start = buffer;
    char *mid = buffer;

    while( ptr != end )
    {
        if(*ptr == ' ')
        {
            mid = ptr;
            *mid = '\0'; // Turn it into a cstring
        }
        else if ( *ptr == '\n' || *ptr == '\t' )
        {
            *ptr = '\0';
            r.url = std::string(start);
            r.rank = (double) atoi(mid + 1);
            res.push_back(r);
            r = Result();
            ++ptr;
            start = ptr;
            continue;
        }
        ++ptr;
    }
    close( sock );

    for(int i = 0; i < res.size(); ++i)
    {
        std::cout << res[i].url << ' '<< res[i].rank << '\n';
    }
    std::cout << "End printing for Node: " << node << '\n';
    return res;
}



std::vector<Result> Server::retrieveSortedDocuments( APESEARCH::string &query ){
    std::vector<Result> documents;
    std::vector< std::future<std::vector<Result>> > futureObjs;
    documents.reserve( SERVERNODES * DOCSPERNODE );
    futureObjs.reserve(SERVERNODES);

    for( size_t i = 0; i < SERVERNODES; ++i )
        futureObjs.emplace_back( threadsPool.submit( callNode, i, query ) );
        //pthread_create(&(rpcPool[i]), NULL, &getRandDocument, (void*) &documents);
    unsigned n = 0;
    for( std::future<std::vector<Result>>& futObj : futureObjs )
       {
        std::cout << "Waiting on node " << n << '\n';
        std::vector<Result> docsOfNode( futObj.get() );
        std::cout << "Got node " << n++ << '\n';
        documents.insert( documents.end(), docsOfNode.begin(), docsOfNode.end() );
        
       } // end for
    sortResults(documents);

    // for(int i = 0; i < documents.size(); ++i)
    // {
    //     std::cout << documents[i].url << ' '<< documents[i].rank << '\n';
    // }
    std::cout << "Finished Sorting\n";
    return documents;
}
