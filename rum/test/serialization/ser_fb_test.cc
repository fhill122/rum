/*
 * Created by Ivan B on 2022/1/7.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>

using namespace rum;


int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}