//
// Created by phil on 10.10.20.
//

#include <gtest/gtest.h>
#include "direct_image_alignment/direct_image_alignment.h"
#include "Log.h"
using namespace testing;
using namespace pd;
using namespace pd::vision;

TEST(ImageAlignmentTest,Align)
{
    Log::init(4);
    ImageAlignment imageAlignment(0,0,7);

    auto camera =std::make_shared<Camera>(1,25,25);
    Eigen::Matrix<int,50,50> imgRef,imgTarget;
    auto frameRef = std::make_shared<Frame>(imgRef,camera);
    auto p3d = std::make_shared<Point3D>(Eigen::Vector3d(10,10,20.0), nullptr);
    auto pImage = frameRef->camera2image(p3d->position());


    EXPECT_LE(pImage.x(),frameRef->width());
    EXPECT_LE(pImage.y(),frameRef->height());

    auto ft = std::make_shared<Feature2D>(pImage,frameRef,p3d);

    VLOG(3) << ft->id() << ":[" << pImage.x() << "," << pImage.y() << "]" << "-> [" << p3d->position().x() << "," << p3d->position().y() << "," <<p3d->position().z() << "]";


            p3d->addFeature(ft);
    frameRef->addFeature(ft);
    auto frameTarget = std::make_shared<Frame>(imgRef,camera);

    auto pose = imageAlignment.align(frameRef,frameTarget);
    //Load reference frame with 3D data
    //Load target frame
    //Align
    p3d->removeFeatures();
    frameRef->removeFeatures();
    //EXPECT_TRUE(pose.translation().norm() > 1);
}

