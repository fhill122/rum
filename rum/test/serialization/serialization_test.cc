/*
 * Created by Ivan B on 2021/12/22.
 */
#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/serialization/native/serializer_native.h>
#include <rum/serialization/native/handwritten.h>

namespace rum{
    // write additional custom type serialization here
}

#include <rum/serialization/native/autoserialize.h>

using namespace std;
using namespace rum;

struct TrivialData1{
    int x, y;
    char data[3];
};

struct TrivialData2{
    int type;
    std::array<TrivialData1, 1024> data;
};

/**
 * This struct cannot be serialized, as it is not trivial and
 * not derived from HandWrittenSerialization
 */
struct NotSerializable{
    int x;
    NotSerializable(){x=4;}
};

/**
 * This is serializable as extra serialization functions are defined
 */
struct Serializable{
    int x;
    Serializable(){x=4;}
};
size_t GetSerializationSize(const Serializable &s) {return sizeof(s.x);}


struct HandWrittenTrivial : HandwrittenSerialization{
    int x;
    int y;

    size_t getSerializationSize() const override {
        return AutoSerializeGetSize(x,y);
    }

    void serialize(void *data) const override {

    }

    void deserialize(const void *data) override {

    }
};

TEST(HandWritten, AutoSerializeTrivial){
    HandWrittenTrivial hw1;
    // not trivial anymore as introduced virtual function
    ASSERT_FALSE(is_trivial<HandWrittenTrivial>::value);
    ASSERT_EQ(hw1.getSerializationSize(), sizeof(hw1.x)+sizeof(hw1.y));
}

TEST(HandWritten, CompilationTest){
    // gtest not able to check compilation error. for now, uncomment to assure compilation failure

    static_assert(is_trivial<TrivialData2>::value);

    NotSerializable nope;
    // AutoSerializeGetSize(nope);  // static assert

    Serializable yep;
    AutoSerializeGetSize(yep);

    std::vector<int> vector1;
    AutoSerializeGetSize(vector1);
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
