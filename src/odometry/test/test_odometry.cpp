//
// Created by phil on 10.10.20.
//

#include <gtest/gtest.h>
#include <opencv2/highgui.hpp>
#include <core/core.h>
#include <utils/utils.h>
#include <lukas_kanade/lukas_kanade.h>
#include "SE3Alignment.h"
#include "IterativeClosestPoint.h"
#include "IterativeClosestPointOcv.h"

#include "RgbdAlignmentOpenCv.h"
#include "Trajectory.h"
#include "Map.h"
#include "MotionPrediction.h"
#include "KeyFrameSelection.h"
#include "Odometry.h"
using namespace testing;
using namespace pd;
using namespace pd::vslam;
using namespace pd::vslam::least_squares;


#define VISUALIZE false

void readAssocTextfile(std::string filename,
                       std::vector<std::string>& inputRGBPaths,
                       std::vector<std::string>& inputDepthPaths,
                       std::vector<Timestamp>& timestamps
                       ) {
    std::string line;
    std::ifstream in_stream(filename.c_str());
    if(!in_stream.is_open()) {  std::runtime_error("Could not open file at: " + filename); }

    while (!in_stream.eof()) {
        std::getline(in_stream, line);
        std::stringstream ss(line);
        std::string buf;
        int c = 0;
        while (ss >> buf) {
            c++;
            if (c == 1) { timestamps.push_back((Timestamp)(std::stod(ss.str())*1e9)); } 
            else if (c == 2) { inputDepthPaths.push_back(buf);} 
            else if (c == 4) { inputRGBPaths.push_back(buf);}
        }
    }
    in_stream.close();
}

class EvaluationOdometry : public Test{
    public:
    EvaluationOdometry()
    {
        if (VISUALIZE)
        {
            LOG_IMG("Residual")->_show = true;
            LOG_IMG("Image")->_show = true;
            LOG_IMG("Depth")->_show = true;
            //LOG_IMG("Template")->_show = true;
            LOG_IMG("ImageWarped")->_show = true;
            LOG_IMG("Weights")->_show = true;
            LOG_IMG("SteepestDescent")->_show = true;
            //LOG_PLT("MedianScaler")->_show = true;
            //LOG_PLT("MedianScaler")->_block = true;
            //LOG_IMG("Residual")->_block = true;
        }
        
        // tum depth format: https://vision.in.tum.de/data/datasets/rgbd-dataset/file_formats
        _datasetPath = TEST_RESOURCE"/rgbd_dataset_freiburg2_desk";
        _cam = std::make_shared<Camera>(525.0,525.0,319.5,239.5);
        readAssocTextfile(_datasetPath + "/assoc.txt",_imgFilenames,_depthFilenames,_timestamps);
        _trajectoryGt = std::make_shared<Trajectory>(utils::loadTrajectory(_datasetPath + "/groundtruth.txt"));

        auto solver = std::make_shared<GaussNewton>(1e-9,100);
        auto loss = nullptr;//std::make_shared<LossTDistribution>(std::make_shared<ScalerTDistribution>());
        _keyFrameSelection = std::make_shared<KeyFrameSelectionIdx>(5);
        _map = std::make_shared<Map>();
        _prediction = std::make_shared<MotionPredictionConstant>();
        _odometry = std::make_shared<OdometryRgbd>(
            30,
            solver, loss, _map);
    }

    FrameRgbd::ShPtr loadFrame(size_t fNo)
    {   
        // tum depth format: https://vision.in.tum.de/data/datasets/rgbd-dataset/file_formats
        return std::make_shared<FrameRgbd>(
            utils::loadImage(_datasetPath + "/" + _imgFilenames.at(fNo)),
            utils::loadDepth(_datasetPath + "/" + _depthFilenames.at(fNo))/5000.0,
            _cam,3,_timestamps.at(fNo));
    }
    protected:
    Odometry::ShPtr _odometry;
    KeyFrameSelection::ShPtr _keyFrameSelection;
    MotionPrediction::ShPtr _prediction;
    Map::ShPtr _map;

    std::vector<std::string> _depthFilenames;
    std::vector<std::string> _imgFilenames;
    std::vector<Timestamp> _timestamps;
    Trajectory::ConstShPtr _trajectoryGt;
    Camera::ConstShPtr _cam;
    std::string _datasetPath;
};

TEST_F(EvaluationOdometry, Sequential)
{
    const int fId0 = 0;
    const int nFrames = _imgFilenames.size();

    Trajectory traj;
    for (int fId = fId0; fId < nFrames; fId++)
    {
        auto fCur = loadFrame(fId);

        fCur->set(*_prediction->predict(fCur->t()));

        _odometry->update(fCur);
            
        fCur->set(*_odometry->pose());

        _prediction->update(_odometry->pose(),fCur->t());

        _keyFrameSelection->update(fCur);
        
        _map->update(fCur, _keyFrameSelection->isKeyFrame());
        traj.append(fCur->t(),std::make_shared<PoseWithCovariance>(fCur->pose().inverse()));
        if(_map->lastKf())
        {
            auto relativePose = algorithm::computeRelativeTransform(_map->lastKf()->pose().pose(),fCur->pose().pose()).inverse();
            auto relativePoseGt = _trajectoryGt->motionBetween(_map->lastKf()->t(),fCur->t())->inverse();
            auto error = (relativePose.inverse() * relativePoseGt.pose()).log();

            std::cout 
                << fId << "/" << nFrames <<": "<< ": " << fCur->pose().pose().log().transpose()
                << "\n Error Translation: " << error.head(3).norm() 
                << "\n Error Angle: " << error.tail(3).norm() << std::endl;
            
        }
        
        utils::writeTrajectory(traj,"trajectory.txt");
    }
    //TODO call evaluation script?
}
