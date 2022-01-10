//
// Created by Ivan B on 2021/4/13.
//

#include <gtest/gtest.h>

#include <rum/common/log.h>
#include <rum/cpprum/node.h>
#include <rum/core/internal/publisher_base_impl.h>
#include <rum/serialization/flatbuffers/serializer_fbs.h>
#include "test_msg/test_point_generated.h"


using namespace std;
using namespace rum;

// void subCb(const test::msg::PointTable* const &msg){
//
// }

void subCb(const test::msg::PointTable &msg){

}

TEST(PublishTest, FlbPub){
    // auto node = Node<SerializerFbs>::init();
    // auto publisher = node->addPublisher<flatbuffers::FlatBufferBuilder>("point");
    //
    // auto builder = make_unique<flatbuffers::FlatBufferBuilder>(50);
    // auto point = test::msg::CreatePointTable(*builder,4,5,6);
    // builder->Finish(point);
    // publisher.pub(move(builder));

    // remove_pointer<test::msg::PointTable*>::type lala;
    // const  test::msg::PointTable* p =
    //         flatbuffers::GetRoot<std::remove_pointer<const test::msg::PointTable*>::type>(nullptr);
    // make_unique<const test::msg::PointTable*>(p);

    // auto sub = node->createSubscriber<const test::msg::PointTable*>("topic", &subCb);

    // auto sub = node->createSubscriber<test::msg::PointTable>("topic", &subCb);

    // NodeBase node_base{};
    // auto sub = node_base.createSubscriber("", nullptr, 0, nullptr, nullptr);
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
