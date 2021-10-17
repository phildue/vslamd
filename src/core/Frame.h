//
// Created by phil on 30.06.21.
//

#ifndef DIRECT_IMAGE_ALIGNMENT_FRAME_H
#define DIRECT_IMAGE_ALIGNMENT_FRAME_H

#include <memory>
#include <vector>
#include <Eigen/Dense>
#include <sophus/se3.hpp>

#include "Feature2D.h"
#include "Camera.h"
#include "core/types.h"
namespace pd{
    namespace vision {

        class Frame {
        public:
            using ShPtr = std::shared_ptr<Frame>;
            using ShConstPtr = std::shared_ptr<const Frame>;

            explicit  Frame(const Image& grayImage, Camera::ConstShPtr camera,uint32_t levels = 1, const Sophus::SE3d& pose = Sophus::SE3d());
            void addFeature(Feature2D::ShPtr ft);
            void removeFeatures();
            void removeFeature(std::shared_ptr< Feature2D> f);

            const Image& grayImage(int level = 0) const;
            Image& grayImage(int level = 0);

            const Image& gradientImage(int level = 0) const;
            Image& gradientImage(int level = 0);

            Eigen::Vector3d world2frame(const Eigen::Vector3d &pWorld) const;
            Eigen::Vector2d world2image(const Eigen::Vector3d &pWorld) const;
            Eigen::Vector3d image2world(const Eigen::Vector2d &pImage, double depth = 1.0) const;
            Eigen::Vector2d camera2image(const Eigen::Vector3d &pCamera) const;
            Eigen::Vector3d image2camera(const Eigen::Vector2d &pImage, double depth = 1.0) const;
            Camera::ConstShPtr camera() const { return _camera;};
            const std::vector<Feature2D::ShConstPtr>& features() const { return _features;}
            const std::vector<Feature2D::ShPtr>& features() { return _features;}
            uint32_t nObservedPoints() const;
            const Sophus::SE3d& pose() const { return _pose;}
            void setPose(const Sophus::SE3d& pose);
            uint32_t width(uint32_t level = 0) const;
            uint32_t height(uint32_t level = 0) const;
            bool isVisible(const Eigen::Vector2d& pImage, double border, uint32_t level = 0) const;
            int levels() const { return _grayImagePyramid.size();};
            ~Frame();
            const std::uint64_t _id;
        private:
            std::vector<Feature2D::ShPtr> _features;
            std::vector<Image> _grayImagePyramid;
            std::vector<Image> _gradientImagePyramid;
            Camera::ConstShPtr _camera;
            Sophus::SE3d _pose,_poseInv;
            static std::uint64_t _idCtr;

        };

    class FrameRGBD : public Frame
    {
    public:
        using ShPtr = std::shared_ptr<FrameRGBD>;
        using ShConstPtr = std::shared_ptr<FrameRGBD>;

        FrameRGBD(const Eigen::MatrixXd& depthMap, const Image& grayImage, Camera::ConstShPtr camera,uint32_t levels = 1, const Sophus::SE3d& pose = Sophus::SE3d());
        const Eigen::MatrixXd& depthMap(int level = 0) const;
        Eigen::MatrixXd& depthMap(int level = 0);

    private:
        std::vector<Eigen::MatrixXd> _depthImagePyramid;

    };
    }}

#endif //MYLIBRARY_FRAME_H
