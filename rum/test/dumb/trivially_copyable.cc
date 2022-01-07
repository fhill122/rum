/*
 * Created by Ivan B on 2021/12/21.
 */
#include <iostream>
#include <type_traits>
#include <array>
#include <memory>

#include <rum/extern/ivtb/log.h>
#include <rum/extern/ivtb/stopwatch.h>

using namespace std;
using namespace ivtb;

struct A { int i; };

class B{
  public:
    A a;
    int i;
};

struct C{
    int* a;
    int b;
    int x[4];
    // std::string f;
    std::array<char,5> data;
};

struct D{
    int x[500000];
};


int main(){
    std::cout << std::boolalpha;
    std::cout << "is_trivially_copyable:" << std::endl;
    std::cout << "int: " << std::is_trivially_copyable<int>::value << std::endl;
    std::cout << "A: " << std::is_trivially_copyable<A>::value << std::endl;
    std::cout << "B: " << std::is_trivially_copyable<B>::value << std::endl;
    std::cout << "C: " << std::is_trivially_copyable<C>::value << std::endl;

    A a{2};
    B b{a, 3};

    B b2;
    cout << b2.a.i <<" "<<b2.i<< endl;
    memcpy(&b2, &b, sizeof(B));
    cout << b2.a.i <<" "<<b2.i<< endl;

    B b3;
    cout << b3.a.i <<" "<<b3.i<< endl;
    char* b_ptr = (char*) (&b);
    std::copy(b_ptr, b_ptr+sizeof(b), (char*)&b3);
    cout << b3.a.i <<" "<<b3.i<< endl;

    Stopwatch stopwatch;
    D d;
    d.x[0] = 100;
    Log::I(__func__, "creation time %.3f ms", stopwatch.passedMs());
    stopwatch.start();
    D d2 = d;
    Log::I(__func__, "copy time %.3f ms", stopwatch.passedMs());
    stopwatch.start();
    D d3 = std::move(d);  // should have no effect, but it is faster, why?
    Log::I(__func__, "move time %.3f ms", stopwatch.passedMs());
    Log::D(__func__, "%d %d %d", d.x[0], d2.x[0], d3.x[0]);
    Log::D(__func__, "%p %p %p", &(d.x[0]), &(d2.x[0]), &(d3.x[0]));

    D d4;
    stopwatch.start();
    memcpy(d4.x, d.x, sizeof(D::x));
    // both memcpy and std::copy is lighting fast when optimization is turned on
    Log::I(__func__, "memcpy time %.3f ms. size: %d, %d",
           stopwatch.passedMs(), sizeof(D::x), sizeof(int));

    stopwatch.start();
    std::copy(begin(d.x), end(d.x), begin(d4.x));
    // well this is fastest, but why?
    Log::I(__func__, "std::copy time %.3f ms. size: %d, %d",
           stopwatch.passedMs(), sizeof(D::x), sizeof(int));

    char* data = new char[sizeof(D::x)];
    stopwatch.start();
    std::copy(begin(d.x), end(d.x), data);
    Log::I(__func__, "heap std::copy on heap time %.3f ms. size: %d, %d",
           stopwatch.passedMs(), sizeof(D::x), sizeof(int));

    stopwatch.start();
    D* d5 = new D(d);
    Log::I(__func__, "heap creation time %.3f ms", stopwatch.passedMs());


    return 0;
}
