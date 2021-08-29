//
// Created by Ivan B on 2021/3/30.
//

#include <rum/extern/Log_cpp14.h>

using namespace std;

class Obj{

};

////////////////////////////////////////////////////////////////

template<typename T>
void serialize(T &t, char** data, int &size){
    Log::V(__func__, "");
}

template<typename T>
T parse(char* data, int &size){
    Log::V(__func__, "");
}

void serializeObj(Obj&t, char** data, int &size){}


/////////////////////////////////////////////////////////////////

template<typename T, typename F>
void publish(T &t, F &&f){
    int size;
    f(t, nullptr, size);
}

/////////////////////////////////////////////////////////////////

decltype(&serialize<Obj>) func2 = &serialize;

template<typename T, typename F = decltype(func2)>
class Publisher{
    F f_;
  public:
    explicit Publisher(F &&f): f_(f){}

    void pub(T &t){
        int size;
        f(t, nullptr, size);
    }

};

template<typename T>
class Publisher2{
    function<void (T&, char**, int&)> f_;
  public:
    template<class F = decltype(&serialize<Obj>)>
    Publisher2(F &&f = &serialize<Obj>): f_(f){}
};

/////////////////////////////////////////////////////////////////

int main(){
    Log::V(__func__ ,"start");

    Obj obj;
    int size;
    serialize(obj, nullptr, size);

    // std::function<void(Obj&, char**, int&)> serialize_obj_f =
    //         [](Obj &obj, char**, int& size_){serialize(obj, nullptr, size_);};
    // serialize_obj_f(obj, nullptr, size_);


    // publishIpc(obj, [](Obj &, char**, int&){printf("inline lambda\n");});
    // publishIpc(obj, serialize_obj_f);


    publish(obj, &serialize<Obj>);

    // this only works in c++17
    // PublisherBaseImpl publisher<Obj, decltype(serialize<obj>)*>{&serialize<Obj>};
    // PublisherBaseImpl publisher<Obj, decltype(serializeObj)*> {&serializeObj};

    decltype(serializeObj)* func1 = &serializeObj;
    decltype(&serializeObj) func2 = &serializeObj;
    Publisher<Obj, decltype(&serializeObj)> publisher2 {&serializeObj};
    Publisher<Obj, decltype(&serialize<Obj>)> publisher3 {&serialize};

    Publisher<Obj> publisher4 {&serialize};

    Publisher2<Obj> pub {&serialize<Obj>};
    Publisher2<Obj> pub2 {};

}
