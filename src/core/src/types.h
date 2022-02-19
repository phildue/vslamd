//
// Created by phil on 07.08.21.
//

#ifndef DIRECT_IMAGE_ALIGNMENT_TYPES_H
#define DIRECT_IMAGE_ALIGNMENT_TYPES_H

#include <Eigen/Dense>
#include <sophus/se3.hpp>


namespace Eigen{
    typedef Eigen::Matrix<double,6,1> Vector6d;
}

namespace pd::vision{
    typedef Eigen::Matrix<std::uint8_t ,Eigen::Dynamic,Eigen::Dynamic> Image;
    typedef Eigen::Matrix<double ,Eigen::Dynamic,Eigen::Dynamic> DepthMap;
    typedef std::uint64_t Timestamp;
    typedef Sophus::SE3d SE3d;
    typedef Eigen::Matrix<std::uint8_t ,Eigen::Dynamic,Eigen::Dynamic> MatXui8;
    typedef Eigen::Matrix<int ,Eigen::Dynamic,Eigen::Dynamic> MatXi;
    typedef Eigen::Matrix<double ,Eigen::Dynamic,Eigen::Dynamic> MatXd;
    typedef Eigen::Matrix<float ,Eigen::Dynamic,Eigen::Dynamic> MatXf;
    typedef Eigen::Matrix<double ,3,3> Mat3d;
    typedef Eigen::Matrix<double ,2,2> Mat2d;
    template<typename Derived, int nRows, int nCols>
    using Mat = Eigen::Matrix<Derived, nRows, nCols>;

    template<int nRows, int nCols>
    using Matd = Eigen::Matrix<double, nRows, nCols>;

    template<int nRows, int nCols>
    using Matf = Eigen::Matrix<double, nRows, nCols>;

    typedef Eigen::VectorXd VecXd;
    typedef Eigen::Vector2d Vec2d;
    typedef Eigen::Vector3d Vec3d;
    typedef Eigen::Vector6d Vec6d;


}

#endif //DIRECT_IMAGE_ALIGNMENT_TYPES_H