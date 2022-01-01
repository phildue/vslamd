//
// Created by phil on 07.08.21.
//
#include <Eigen/Dense>
#include <opencv2/imgproc.hpp>
#include <opencv4/opencv2/core/eigen.hpp>
#include <opencv4/opencv2/highgui.hpp>

#include "Exceptions.h"
#include "core/Frame.h"
#include "core/Point3D.h"
#include "core/algorithm.h"
#include "visuals.h"
#include "Log.h"
namespace pd {
namespace vision {
namespace vis {

cv::Mat drawFeatures(std::shared_ptr<const Frame> frame, int radius,
                     bool gradient) {
  cv::Mat mat;
  if (gradient) {
    cv::eigen2cv(frame->gradientImage(), mat);
  } else {
    cv::eigen2cv(frame->grayImage(), mat);
  }

  if (mat.cols == 0 || mat.rows == 0) {
    throw pd::Exception("Image is empty!");
  }

  for (const auto &ft : frame->features()) {
    cv::rectangle(
        mat,
        cv::Point(ft->position().x() - radius, ft->position().y() - radius),
        cv::Point(ft->position().x() + radius, ft->position().y() + radius),
        cv::Scalar(255, 255, 255));
  }

  return mat;
}

cv::Mat drawFeaturesWithPoints(std::shared_ptr<const Frame> frame, int radius) {

  cv::Mat mat;
  cv::eigen2cv(frame->gradientImage(), mat);

  if (mat.cols == 0 || mat.rows == 0) {
    throw pd::Exception("Image is empty!");
  }

  for (const auto &ft : frame->features()) {
    if (ft->point()) {
      cv::circle(mat, cv::Point(ft->position().x(), ft->position().y()), radius,
                 cv::Scalar(255, 255, 255));
    } else {
      cv::rectangle(
          mat,
          cv::Point(ft->position().x() - radius, ft->position().y() - radius),
          cv::Point(ft->position().x() + radius, ft->position().y() + radius),
          cv::Scalar(255, 255, 255));
    }
  }
  return mat;
}

cv::Mat drawFrame(std::shared_ptr<const Frame> frame) {

  cv::Mat mat;
  cv::eigen2cv(frame->grayImage(), mat);
  return mat;
}

/*       void Log::logFrame(std::shared_ptr<const FrameRGBD> frame, int level,
const std::string &name) { if (level <= _logLevel)
           {
               cv::Mat matRgb;
               cv::eigen2cv(frame->grayImage(),matRgb);
               const Eigen::MatrixXd matImage =
algorithm::normalize(frame->depthMap()) * 255.0; cv::Mat matD;
//                cv::eigen2cv(matImage.cast<std::uint8_t>(),matD);
//                const cv::Mat matRgbd = cv::hstack(matRgb,matD);
               logMat(matRgb, level, name);
           }

       }*/

cv::Mat drawReprojection(std::shared_ptr<const pd::vision::Frame> frame0,
                         std::shared_ptr<const pd::vision::Frame> frame1,
                         int radius) {

  cv::Mat mat;
  cv::eigen2cv(frame1->grayImage(), mat);

  if (mat.cols == 0 || mat.rows == 0) {
    throw pd::Exception("Image is empty!");
  }

  for (const auto &ft : frame0->features()) {
    if (ft->point()) {
      auto reprojectedPosition = frame1->world2image(ft->point()->position());
      cv::rectangle(mat,
                    cv::Point(reprojectedPosition.x() - radius,
                              reprojectedPosition.y() - radius),
                    cv::Point(reprojectedPosition.x() + radius,
                              reprojectedPosition.y() + radius),
                    cv::Scalar(255, 255, 255));
    }
  }

  return mat;
}


cv::Mat drawAsImage(const Eigen::MatrixXd& mat)
{
    return drawMat((algorithm::normalize(mat)*255).cast<uint8_t>());
}
cv::Mat drawMat(const Image &matEigen) {
  cv::Mat mat;
  cv::eigen2cv(matEigen, mat);

  return mat;
}

void PlotLevenbergMarquardt::plot() const
{
    plt::figure();
    plt::subplot(1,4,1);
    plt::title("$Chi^2$");
    std::vector<double> chi2v(_chi2.data(), _chi2.data() + _chi2.rows() * _chi2.cols());
    plt::named_plot("$Chi^2$",chi2v);
    plt::legend();

    plt::subplot(1,4,2);
    plt::title("$\\Delta Chi^2$");
    std::vector<double> chi2predv(_chi2pred.data(), _chi2pred.data() + _chi2pred.rows() * _chi2pred.cols());
    plt::named_plot("$\\hat{\\Delta Chi^2}$",chi2predv);

    plt::subplot(1,4,3);
    plt::title("$\\lambda$");
    std::vector<double> lambdav(_lambda.data(), _lambda.data() + _lambda.rows() * _lambda.cols());
    plt::named_plot("$\\lambda$",lambdav);
    plt::legend();

    plt::subplot(1,4,4);
    plt::title("$\\Delta X$");
    std::vector<double> stepsizev(_stepSize.data(), _stepSize.data() + _stepSize.rows() * _stepSize.cols());
    plt::named_plot("$\\Delta X$",stepsizev);
    plt::legend();
  

}
std::string PlotLevenbergMarquardt::csv() const 
{
    return "";
}

} // namespace vis
} // namespace vision
} // namespace pd
