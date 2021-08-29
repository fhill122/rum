//
// Created by Ivan B on 2021/3/21.
//

#include <rum/extern/zmq/zmq.hpp>

#include <unistd.h>

constexpr char addr_a[] = "tcp://127.0.0.1:6666";
constexpr char addr_b[] = "tcp://127.0.0.1:6667";

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

    zmq::context_t context_a(1);
    zmq::socket_t node_a(context_a, ZMQ_ROUTER);
    node_a.bind(addr_a);

    zmq::context_t context_b(1);
    zmq::socket_t node_b(context_b, ZMQ_ROUTER);
    node_b.bind(addr_b);


    node_a.connect(addr_b);
    node_b.connect(addr_a);

    usleep(1e6);

    s_send(node_a, "lala");
    usleep(1e6);

    printf("call receive\n");
    auto recv = s_recv(node_b);

    printf("b received: %s\n", recv.c_str());

}
