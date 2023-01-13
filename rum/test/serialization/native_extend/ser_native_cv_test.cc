/*
 * Created by Ivan B on 2023/1/11.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/cpprum/serialization/native/extend/cv_types.h>
#include <rum/cpprum/serialization/native/serializer_native.h>


using namespace std;
using namespace rum;

bool IsEqual(const cv::Mat &mat1, const cv::Mat &mat2){
    AssertLog(mat1.channels() == mat2.channels(), "");
    AssertLog(mat1.channels()<=4, "");
    return (sum(mat1 != mat2) == cv::Scalar(0,0,0,0));
}

TEST(CvSerialization, MatTest){
    SerializerNative serializer;
    shared_ptr<cv::Mat> mat_sp = make_shared<cv::Mat> (6,4, CV_8UC3);
    for (int r = 0; r < mat_sp->rows; ++r) {
        for (int c = 0; c < mat_sp->cols; ++c) {
            mat_sp->at<uchar>(r,c) = (uchar)std::rand();
        }
    }
    shared_ptr<const Message> data = serializer.serialize((shared_ptr<const cv::Mat>)mat_sp);
    unique_ptr<cv::Mat> mat_deser = serializer.deserialize<cv::Mat>(data, serializer.Protocol());
    ASSERT_TRUE(IsEqual(*mat_sp, *mat_deser));

    mat_sp->at<uchar>(0,0) += 1;
    ASSERT_FALSE(IsEqual(*mat_sp, *mat_deser));
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
