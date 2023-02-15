/*
 * Created by Ivan B on 2023/2/7.
 */

#ifndef RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_EIGEN_TYPES_H_
#define RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_EIGEN_TYPES_H_

#include <Eigen/Core>

#include "rum/cpprum/serialization/native/handwritten.h"

namespace rum {

struct EigenHeader{
    char type;
    int rows;
    int cols;
    int storage_option;
};

template <typename S>
char GetScalarType(){
    if constexpr (std::is_same<S,int>::value){
        return 1;
    }
    if constexpr (std::is_same<S,float>::value){
        return 2;
    }
    if constexpr (std::is_same<S,double>::value){
        return 3;
    }

    static_assert(std::is_same<S,int>::value ||
                  std::is_same<S,float>::value ||
                  std::is_same<S,double>::value,
                  "scalar type not supported yet");

    return 0;
}

template<typename Scalar, int Rows, int Cols, int Options>
size_t GetSerializationSize(const Eigen::Matrix<Scalar, Rows, Cols, Options> &mat){
    return sizeof(EigenHeader) + sizeof(Scalar)*mat.rows()*mat.cols();
}

template<typename Scalar, int Rows, int Cols, int Options>
void Serialize(char* buffer, const Eigen::Matrix<Scalar, Rows, Cols, Options> &mat){
    EigenHeader header;
    header.type = GetScalarType<Scalar>();
    header.rows = mat.rows();
    header.cols = mat.cols();
    header.storage_option = mat.Options;
    AutoSerialize(buffer, header);
    buffer += AutoGetSerializationSize(header);

    Eigen::Map<Eigen::Matrix<Scalar, Rows, Cols, Options>> mat_map((Scalar*)buffer, mat.rows(), mat.cols());
    mat_map = mat;
}

template<typename Scalar, int Rows, int Cols, int Options>
void Deserialize(const char* buffer, Eigen::Matrix<Scalar, Rows, Cols, Options> &mat){
    EigenHeader header;
    AutoDeserialize(buffer, header);
    buffer += AutoGetSerializationSize(header);
    AssertLog(GetScalarType<Scalar>() == header.type, "Different scalar type");
    AssertLog(Rows == Eigen::Dynamic ||
              Rows == header.rows, "Row size mismatch");
    AssertLog(Cols == Eigen::Dynamic ||
              Cols == header.cols, "Col size mismatch");
    AssertLog(mat.Options == header.storage_option, "Different storage option");

    Eigen::Map<Eigen::Matrix<Scalar, Rows, Cols, Options>> mat_map((Scalar*)buffer, header.rows, header.cols);
    mat = mat_map;
}


}

#endif //RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_EIGEN_TYPES_H_
