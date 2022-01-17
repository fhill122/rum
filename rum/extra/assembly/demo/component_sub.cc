/*
 * Created by Ivan B on 2022/1/14.
 */

#include <rum/extra/assembly/component.h>
#include <rum/cpprum/rum.h>
#include <rum/serialization/native/serializer_native.h>
#include "message.h"

using namespace std;

class DemoComponentSub : rum::Component{

    int main(int argc, char **argv) override {
        rum::log.setLogLevel(rum::Log::Destination::Std, rum::Log::Level::w);
        rum::Log::I("sub", "sub start here");

        bool res = rum::Init();
        rum::AddSubscriber<Point3d, rum::SerializerNative>("Point", DemoComponentSub::SubCallback);
        while(true) this_thread::sleep_for(1s);
        return 0;
    }

    static void SubCallback(const shared_ptr<const Point3d> &point){
        rum::Log::V("sub", "received a point of %.3f %.3f %.3f. address: %p",
               point->x, point->y, point->z, point.get());
    }
};

RUM_EXPORT_COMPONENT(DemoComponentSub)
