//
// Created by phil on 07.08.21.
//
#include "feature_extraction/feature_extraction.h"
#include "StereoTracking.h"
#include "core/Point3D.h"
#include "core/Feature2D.h"
#include "core/Frame.h"
#include "utils/Log.h"
namespace pd { namespace  vision{

    Sophus::SE3d StereoTracking::align(const Image &img, const Eigen::MatrixXd &depthMap, Timestamp t) {

        VLOG(5) << "Align";

        Sophus::SE3d relativePose;
        auto curFrame = std::make_shared<Frame>(img,_camera,_config.levelMax + 1 );
        Log::logFrame(curFrame,4,"System");
        if ( _firstFrame )
        {
            VLOG(5) << "First frame.";
            _firstFrame = false;
            _frameRef = curFrame;

        }else {
            VLOG(5) << "New frame..";

            _tracker->track(_frameRef,curFrame);

            _frameRef = curFrame;

        }
        _featureExtractor->extractFeatures(_frameRef);


        int nPoints = 0;
        for(const auto& ft : _frameRef->features())
        {
            int i = ft->position().x();
            int j = ft->position().y();
            const double& d = depthMap(i,j);
            if ( d > 0 )
            {
                const auto p3d = _frameRef->image2world(ft->position(), d);
                ft->point() = std::make_shared<Point3D>(p3d,ft);
                nPoints ++;
            }
        }
        VLOG(5) << "Found: ["<< nPoints << "] 3d points.";
        return relativePose;
    }

    StereoTracking::StereoTracking(const StereoTracking::Config &config)
    : _config(config)
    , _featureExtractor(std::make_shared<FeatureExtractionOpenCv>(config.desiredFeatures))
    , _tracker(std::make_shared<Tracker3d>())
    , _firstFrame(true)
    , _camera(std::make_shared<Camera>(config.fx,config.cx,config.cy))
    {
        VLOG(5) << "Stereotracking constructed";
    }
}}