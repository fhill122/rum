/*
 * Created by Ivan B on 2022/1/14.
 */

#include <rum/extra/assembly/component.h>
#include <rum/cpprum/rum.h>
#include <rum/serialization/native/serializer_native.h>
#include "message.h"

using namespace std;

class DemoComponentPub : rum::Component{
    int main(int argc, char **argv) override {
        rum::log.setLogLevel(rum::Log::Destination::Std, rum::Log::Level::w);
        rum::Log::I("pub", "pub start here");

        rum::Init();
        auto pub = rum::CreatePublisher<Point3d, rum::SerializerNative>("Point");

        while(true){
            unique_ptr<Point3d> point = make_unique<Point3d>();
            point->x = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/5));
            point->y = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/5));
            point->z = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/5));
            rum::Log::I("pub", "publishing a point of %.3f %.3f %.3f. address: %p",
                   point->x, point->y, point->z, point.get());
            pub->pub(move(point));
            this_thread::sleep_for(1s);
        }
        return 0;
    }
};

RUM_EXPORT_COMPONENT(DemoComponentPub)
