//
// Created by Ivan B on 2021/3/31.
//

#include <rum/extern/Log_cpp14.h>

template<typename T>
class Serializer {
  public:
    template<typename P>
    void serialize(const P& p, char **data, size_t &size) {
        ((T*)this)-> template serialize<P>(p, data, size);
    }
};


class serializer_proto : public Serializer<serializer_proto>{
  public:
    template<class P>
    void serialize(const P& p, char **data, size_t &size) {
        size = 1;
        *data = new char[size];
        p.SerializeToArray(*data);
        Log::V(__func__, "protobuf serialization");
    }
};


// template <typename S>
// struct NodeBase{
//     S s_;
//     explicit NodeBase(S &&s) : s_(s){}
//     explicit NodeBase(S s) : s_(s){}
//
//     template<class S>
//     void pub(const S& t){
//         char *data_;
//         size_t size_;
//         s_.serialize(t, &data_, size_);
//     }
// };


template <class S = serializer_proto>
struct Node{
    Serializer<S> s_ = Serializer<serializer_proto>();
    explicit Node(Serializer<S> s) : s_(s){}
    explicit Node() = default;

    template<class T>
    void pub(const T& t){
        char *data;
        size_t size;
        s_.serialize(t, &data, size);
    }
};


struct Obj{
    void SerializeToArray(char* data) const{}
};


int main(){
    Obj obj;

    Serializer<serializer_proto> serializer;
    Node<serializer_proto> node(serializer);
    node.pub(obj);


    Node<serializer_proto> node2;
    node2.pub(obj);

    Node<> node3;
    node3.pub(obj);

    // c++17
    // NodeBase node4
}
