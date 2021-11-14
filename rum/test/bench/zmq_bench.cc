/*
 * Created by Ivan B on 2021/11/12.
 */

#include <rum/extern/zmq/zmq.hpp>
#include <rum/extern/ivtb/log.h>
#include <rum/extern/ivtb/stopwatch.h>
#include <thread>

constexpr char tcp_addr[] = "tcp://127.0.0.1:6666";
constexpr char ipc_addr[] = "ipc:///tmp/rum_zmq_bench_test";

using namespace std;
using namespace zmq;
using namespace ivtb;


double PingPong(const string &addr, int count, size_t msg_size){
    static zmq::context_t context(1);

    msg_size = max(sizeof(int), msg_size);

    socket_t node_a(context, ZMQ_PUB);
    node_a.bind(addr);
    socket_t node_b(context, ZMQ_SUB);
    node_b.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    node_b.connect(addr);
    this_thread::sleep_for(500ms);

    Stopwatch stopwatch;
    for (int i=0; i<count; ++i){
        zmq::message_t msg(msg_size);
        *(int*)msg.data() = i;
        node_a.send(msg);

        zmq::message_t msg_received;
        bool ok = node_b.recv(&msg_received);
        int received_num = *(int*)msg_received.data();
        AssertLog(i==received_num, "number not checked");
    }
    return stopwatch.passedMs();
}

double PingPongMultiMsg(const string &addr, int count, size_t msg_size){
    static zmq::context_t context(1);

    msg_size = max(sizeof(int), msg_size);

    socket_t node_a(context, ZMQ_PUB);
    node_a.bind(addr);
    socket_t node_b(context, ZMQ_SUB);
    node_b.setsockopt(ZMQ_SUBSCRIBE, NULL, 0);
    node_b.connect(addr);
    this_thread::sleep_for(500ms);

    Stopwatch stopwatch;
    for (int i=0; i<count; ++i){
        zmq::message_t msg(sizeof(int));
        *(int*)msg.data() = i;
        node_a.send(msg,zmq::send_flags::sndmore);
        zmq::message_t msg_2(msg_size - sizeof(int));
        node_a.send(msg_2, zmq::send_flags::none);

        zmq::message_t msg_received;
        bool ok = node_b.recv(&msg_received);
        int received_num = *(int*)msg_received.data();
        AssertLog(ok && i==received_num, "number not checked");

        zmq::message_t msg_received_2;
        ok = node_b.recv(&msg_received_2);
    }
    return stopwatch.passedMs();
}


int main(int argc, char** argv){
    constexpr int kCount = 10000;
    double t;

    // 0.045 ms
    t = PingPong(tcp_addr,kCount,0);
    Log::I(__func__, "ping pong takes %f ms on average", t/kCount);

    // 0.034 ms
    t = PingPong(ipc_addr,kCount,0);
    Log::I(__func__, "ping pong takes %f ms on average", t/kCount);

    // well this is weird, tcp is faster on large msg?
    // 0.21 ms
    t = PingPong(tcp_addr,kCount,1e6);
    Log::I(__func__, "ping pong takes %f ms on average", t/kCount);

    // 0.24 ms
    t = PingPong(ipc_addr,kCount,1e6);
    Log::I(__func__, "ping pong takes %f ms on average", t/kCount);

    /* multi parge msg, timing almost same*/

    t = PingPongMultiMsg(tcp_addr, kCount, 0);
    Log::I(__func__, "multi msg ping pong takes %f ms on average", t/kCount);

    t = PingPongMultiMsg(ipc_addr, kCount, 0);
    Log::I(__func__, "multi msg ping pong takes %f ms on average", t/kCount);

    t = PingPongMultiMsg(tcp_addr,kCount,1e6);
    Log::I(__func__, "multi msg ping pong takes %f ms on average", t/kCount);

    t = PingPongMultiMsg(ipc_addr,kCount,1e6);
    Log::I(__func__, "multi msg ping pong takes %f ms on average", t/kCount);
}

