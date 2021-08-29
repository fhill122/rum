//
// Created by Ivan B on 2021/4/11.
//

#include <memory>

using namespace std;
int main(int argc, char **argv){
    int x=10;

    shared_ptr<int> *sptr_p = new shared_ptr<int>( new int(move(x)));
    printf("count %ld\n", sptr_p->use_count());

    shared_ptr<int> copy = *sptr_p;
    printf("count %ld\n", sptr_p->use_count());

    delete sptr_p;
    printf("count %ld\n", sptr_p->use_count());

    printf("value = %d\n", *copy);

    // wrong
    printf("value = %d\n", x);  // it always print 10? why?
    printf("value = %d\n", **sptr_p);  // it always print 10? why?

    return 0;
}