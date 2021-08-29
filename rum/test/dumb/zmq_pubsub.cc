//
// Created by Ivan B on 2021/3/21.
//

#include <rum/extern/zmq/zmq.hpp>

#include <unistd.h>

constexpr char tcp_addr[] = "tcp://127.0.0.1:6666";
constexpr char ipc_addr[] = "inproc://test.ipc";


//  Convert string to 0MQ string and send to socket
inline static bool
s_send (zmq::socket_t & socket, const std::string & string, int flags = 0) {
    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, flags);
    return (rc);
}

//  Receive 0MQ string from socket and convert into string
inline static std::string
s_recv (zmq::socket_t & socket, int flags = 0) {

    zmq::message_t message;
    socket.recv(&message, flags);

    return std::string(static_cast<char*>(message.data()), message.size());
}


int main(int argc, char** argv){
    // create nodes
    zmq::context_t context_a(1);
    zmq::socket_t node_a(context_a, ZMQ_PUB);
    node_a.bind(tcp_addr);
    node_a.bind(ipc_addr);


    zmq::context_t context_b(1);
    zmq::socket_t node_b(context_a, ZMQ_SUB);
    node_b.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    // node_b.bindTcpRaw("tcp://127.0.0.1:6667");
    // node_b.bindTcpRaw(tcp_addr);

    // usleep(1e5);
    // node_a.connectRaw(tcp_addr);
    node_b.connect(tcp_addr);
    node_b.connect(ipc_addr);
    usleep(1e5);

    bool res = s_send(node_a, "sent from a");
    
    // s_send(node_c, "sent from c");
    printf("message sent: %d\n", res);
    // usleep(1e5);

    printf("received in b: %s\n", s_recv(node_b).c_str());
    printf("received in b: %s\n", s_recv(node_b).c_str());
    // printf("received in b: %s\n", s_recv(node_b).c_str());
}
