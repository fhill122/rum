/*
 * Created by Ivan B on 2022/2/11.
 */

#ifndef RUM_CPPRUM_NODE_H_
#define RUM_CPPRUM_NODE_H_

#include "rum/serialization/flatbuffers/serializer_fbs.h"

namespace rum{

struct DefaultSerializer{
    // todo ivan. use unique_ptr
    std::shared_ptr<void> serializer;
};


class Node{
    // but how do we cast back to its original type?
    std::shared_ptr<void> serializer;

  public:
    template<class DefSerializerT>
    static bool Init(){
        if constexpr (!std::is_same_v<DefSerializerT, void>) {
            // serializer = std::make_shared<Serializer<DefSerializerT>>();
        }
    }

    template<class SerializerT>
    int getItcCallback(){
        // if serializer is not null, easy

        // if serializer is null

    }

};

}

#endif //RUM_CPPRUM_NODE_H_
