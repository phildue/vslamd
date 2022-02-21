#ifndef VSLAM_IMAGE_KERNEL_H__
#define VSLAM_IMAGE_KERNEL_H__
#include <Eigen/Dense>
#include "types.h"
namespace pd::vision{
template<typename T>
class Kernel2d{
      public:
      
        static Eigen::Matrix<T,-1,-1> sobelY(){
                 Eigen::Matrix<T, 3, 3> sobel;
                sobel << -1, -2, -1,
                  0, 0, 0,
                 +1, +2, +1;
                 return sobel;
        }
        static Eigen::Matrix<T,-1,-1> sobelX(){
                 Eigen::Matrix<T, 3, 3> sobel;
                sobel << -1, 0, 1,
                  -2, 0, 2,
                 -1, 0, 1;
                 return sobel;
        }
        static Eigen::Matrix<T,-1,-1> scharrY(){
                 Eigen::Matrix<T, 3, 3> scharr;
                scharr << -3, -10, -3,
                  0, 0, 0,
                 +3, +10, +3;
                 return scharr;
        }
        static Eigen::Matrix<T,-1,-1> scharrX(){
                 Eigen::Matrix<T, 3, 3> scharr;
                scharr << -3, 0, 3,
                  -10, 0, 10,
                 -3, 0, 3;
                 return scharr;
        }
};
}
#endif