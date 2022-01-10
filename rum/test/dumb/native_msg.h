/*
 * Created by Ivan B on 2022/1/10.
 */

#ifndef RUM_TEST_DUMB_NATIVE_MSG_H_
#define RUM_TEST_DUMB_NATIVE_MSG_H_

#include <string>
#include <vector>
#include <array>

#include <rum/serialization/native/common_types.h>
#include <rum/serialization/native/serializer_native.h>


struct Pose{
    std::array<float,3> position;
    std::array<float, 4> quaternion;
};

struct PoseStamped{
    double time;
    Pose pose;
};


struct Image : rum::HandwrittenSerialization{
    enum ImageEncode{
        BGR,
        RGB,
        GRAY,
        JPEG,
        YUV
    };

    int w;
    int h;
    int c;
    ImageEncode encode;
    std::string frame_id;
    std::vector<char> data;
    std::array<float,4> intrinsics;

    bool isValid(){
        return w*h*c == data.size();
    }

    AUTO_SERIALIZE_MEMBERS(w,h,c,encode,frame_id,data,intrinsics)
};

#endif //RUM_TEST_DUMB_NATIVE_MSG_H_
