/*
 * socket.h
 *
 * Header file for the server class.
 */
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <sys/types.h>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sstream>
using std::istringstream;
#include <utility>
#include <mutex>

using std::string;
using std::vector;

extern std::mutex cout_lock;

/*
 * Socket Servicer: repesents an object that 
*/
class Socket {
public:
    Socket(int port_number);
    int create_init_socket(int port_number);
    int get() const { return listen_socket; }
    int get_connection_socket();
    static void handle_error(bool condition, const char *msg);
    bool receive_request(char * const buffer_ptr, const int msg_sock, const unsigned length_of_req, const unsigned bytes_recvd_so_far);
    void send_response(const string& password, const int sock, const unsigned session, const unsigned seq, const string& data);

    //! Guys make sure this size is sufficent if the exam asks for a new request/new header format
    static const size_t MAX_SIZE = 8192;

private:

    // File handler (or descripter) used by the OS
    const int listen_socket; // Holds the descriptor for the socket (which OS will handle)
};

#endif /* _SERVER_H_ */
