/*
 * Created by Ivan B on 2023/1/11.
 */

#include <gtest/gtest.h>
#include <rum/common/log.h>

#include <rum/cpprum/serialization/native/extend/eigen_types.h>
#include <rum/cpprum/serialization/native/serializer_native.h>


using namespace std;
using namespace rum;
using namespace Eigen;


template<typename Scalar, int Rows, int Cols, int Options,
         typename Scalar2 = Scalar, int Rows2 = Rows, int Cols2 = Cols, int Options2 = Options>
void MatSerial(int rows, int cols){
    using MatT = Matrix<Scalar, Rows, Cols, Options>;
    using MatT2 = Matrix<Scalar2, Rows2, Cols2, Options2>;
    SerializerNative serializer;

    shared_ptr<MatT> mat = make_shared<MatT>(rows, cols);
    mat->setRandom();

    shared_ptr<const Message> data = serializer.serialize((shared_ptr<const MatT>)mat);
    unique_ptr<MatT2> mat_deser = serializer.deserialize<MatT2>(data, serializer.Protocol());
    ASSERT_TRUE(mat->isApprox(*mat_deser));

    (*mat)(0,0) += 1;
    ASSERT_FALSE(mat->isApprox(*mat_deser));
}


TEST(EigenMatrixSerialization, MatBasicTest){
    // int with different alignmentß
    MatSerial<int,4,4,RowMajor>(4,4);
    MatSerial<int,4,4,ColMajor>(4,4);

    // other types
    MatSerial<double,3,4,ColMajor>(3,4);
    MatSerial<float,3,4,ColMajor>(3,4);

    // dynamic size check
    MatSerial<int,Dynamic,Dynamic,ColMajor>(4,4);
    MatSerial<double,4,4,ColMajor, double,Dynamic,Dynamic>(4,4);
    MatSerial<double,4,4,ColMajor, double,4,Dynamic>(4,4);
    MatSerial<double,Dynamic,Dynamic,ColMajor, double,4,Dynamic>(4,4);

    // vector
    MatSerial<int,4,1,ColMajor>(4,1);
}


TEST(EigenMatrixSerialization, MatErrTest){
    SerializerNative serializer;

    shared_ptr<const Matrix4d> mat = make_shared<const Matrix4d>(Matrix4d::Identity());
    shared_ptr<const Message> data = serializer.serialize(mat);

    // different scalar
    ASSERT_DEATH(serializer.deserialize<Matrix4f>(data, serializer.Protocol()), "");

    // different storage option
    using T1 = Matrix<double,4,4,RowMajor>;
    ASSERT_DEATH(serializer.deserialize<T1>(data, serializer.Protocol()), "");

    // incompatible dimension
    ASSERT_DEATH(serializer.deserialize<Matrix3d>(data, serializer.Protocol()), "");
    using T2 = Matrix<double,Dynamic,5>;
}

int main(int argc, char **argv){
    rum::log.setLogLevel(Log::Destination::Std, Log::Level::v);

    ::testing::InitGoogleTest(&argc, argv);
    int res =  RUN_ALL_TESTS();

    rum::printer.i(__func__, "all done");
    return res;
}
