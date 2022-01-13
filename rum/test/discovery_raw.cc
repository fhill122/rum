//
// Created by Ivan B on 2021/4/4.
//

#include <gtest/gtest.h>

#include <rum/core/internal/master.h>
#include <rum/core/internal/sub_container.h>
#include <rum/core/internal/publisher_base_impl.h>
#include <rum/common/log.h>
#include <rum/common/common.h>
#include <rum/core/msg/rum_header_generated.h>

using namespace std;
using namespace rum;


TEST(DicoveryTest, Discovery){
    SUCCEED();
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}
