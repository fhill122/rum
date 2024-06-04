/*
 * Created by Ivan B on 2023/1/10.
 */

#ifndef RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_CV_TYPES_H_
#define RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_CV_TYPES_H_

#include "rum/cpprum/serialization/native/handwritten.h"
#include <opencv2/opencv.hpp>

namespace rum{

/*
 * cv::KeyPoint, cv::Point, etc are trivially copyable
 */


/*
 * cv::Mat
 */

struct CvMatHeader{
    int rows;
    int cols;
    // int channels;
    int type;
};

inline size_t GetSerializationSize(const cv::Mat &mat){
    return sizeof(CvMatHeader) + mat.elemSize()*mat.cols*mat.rows;
}

inline void Serialize(char* buffer, const cv::Mat &mat){
    CvMatHeader header{mat.rows, mat.cols, mat.type()};
    AutoSerialize(buffer, header);
    buffer += AutoGetSerializationSize(header);

    // Data
    if (mat.isContinuous()){
        std::copy(mat.data, mat.data + mat.cols*mat.rows*mat.elemSize(), buffer);
    }
    else{
        const int row_size = mat.elemSize() * mat.cols;
        for (int r = 0; r < mat.rows; ++r){
            const char* row_data = mat.ptr<char>(r);
            std::copy(row_data, row_data + row_size, buffer);
            buffer += row_size;
        }
    }
}

inline void Deserialize(const char* buffer, cv::Mat &mat){
    CvMatHeader header;
    AutoDeserialize(buffer, header);
    buffer += AutoGetSerializationSize(header);

    mat = cv::Mat(header.rows, header.cols, header.type);
    CV_Assert(mat.isContinuous());  // should always hold no matter the dimension
    std::copy(buffer, buffer + mat.cols*mat.rows*mat.elemSize(), mat.data);
}


}

#endif //RUM_CPPRUM_SERIALIZATION_NATIVE_EXTEND_CV_TYPES_H_
