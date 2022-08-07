#include "FeatureTracking.h"
#include <set>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/ocl.hpp>
#include <Eigen/Dense>
#include <opencv4/opencv2/core/eigen.hpp>
namespace pd::vslam{

        std::vector<Point3D::ShPtr> FeatureTracking::track(FrameRgbd::ShPtr frameCur, std::vector<FrameRgbd::ShPtr>& framesRef)
        {
                extractFeatures(frameCur);
                return match(frameCur,selectCandidates(frameCur,framesRef));
        }


        void FeatureTracking::extractFeatures(FrameRgbd::ShPtr frame) const
        {
                cv::Mat image;
                cv::eigen2cv(frame->intensity(),image);
                cv::Ptr<cv::DescriptorExtractor> extractor = cv::ORB::create(_nFeatures);
                cv::Mat desc;
                std::vector<cv::KeyPoint> kpts;
                extractor->detectAndCompute(image,cv::Mat(),kpts,desc);
                MatXd descriptors;
                cv::cv2eigen(desc,descriptors);
                cv::Mat imgout;
                cv::cvtColor(image,imgout,cv::COLOR_GRAY2BGR);
                /*cv::drawKeypoints(image,kpts,imgout);
                cv::imshow("KeyPoints",imgout);
                cv::waitKey(0);*/
                std::vector<Feature2D::ShPtr> features;
                features.reserve(kpts.size());
                for (size_t i = 0U; i < kpts.size(); ++i)
                {
                        const auto& kp = kpts[i];
                        frame->addFeature(std::make_shared<Feature2D>(Vec2d(kp.pt.x,kp.pt.y),frame,kp.octave,kp.response,descriptors.row(i)));
                }
        }

        std::vector<Point3D::ShPtr> FeatureTracking::match(FrameRgbd::ShPtr frameCur,const std::vector<Feature2D::ShPtr>& featuresRef) const
        {
                auto featuresCur = frameCur->features();
                const auto dim = featuresCur[0]->descriptor().rows();
                MatXd desc1E = MatXd::Zero(featuresCur.size(),dim);
                MatXd desc2E = MatXd::Zero(featuresRef.size(),dim);
                for(int i = 0; i < desc1E.rows(); i++)
                {
                        desc1E.row(i) = featuresCur[i]->descriptor();
                }
                for(int i = 0; i < desc2E.rows(); i++)
                {
                        desc2E.row(i) = featuresRef[i]->descriptor();
                }
                cv::Mat desc1;
                cv::Mat desc2;
                cv::eigen2cv(desc1E,desc1);
                cv::eigen2cv(desc2E,desc2);
                desc1.convertTo(desc1,CV_8U);
                desc2.convertTo(desc2,CV_8U);

                std::vector<cv::DMatch> matches;  
                cv::BFMatcher desc_matcher(cv::NORM_HAMMING, true);
                desc_matcher.match(desc1, desc2, matches);
                std::vector<Point3D::ShPtr> points;
                points.reserve(matches.size());
                for (const auto& m : matches)
                {
                        auto fRef = featuresRef[m.trainIdx];
                        auto fCur = featuresCur[m.queryIdx];
                        if (fRef->point()){ 
                                fCur->point() = fRef->point();
                                fRef->point()->addFeature(fCur);
                        }else{
                                std::vector<Feature2D::ShPtr> features = {fCur,fRef};
                                //TODO get 3d point
                                fCur->point() = std::make_shared<Point3D>(frameCur->p3d(fCur->position().y(),fCur->position().x()),features);
                                fRef->point() = fCur->point();
                        }
                        points.push_back(fCur->point());
                }
                return points;
        }
        std::vector<Feature2D::ShPtr> FeatureTracking::selectCandidates(FrameRgbd::ConstShPtr frameCur, std::vector<FrameRgbd::ShPtr>& framesRef) const
        {
                std::set<std::uint64_t> pointIds;
                
                std::vector<Feature2D::ShPtr> candidates;
                candidates.reserve(framesRef.size() * framesRef[0]->features().size());
                //TODO sort frames from new to old
                const double border = 5.0;
                for(auto& f : framesRef)
                {
                        for (auto ft : f->features())
                        {
                                if(!ft->point()){ candidates.push_back(ft);}
                                else if(std::find(pointIds.begin(),pointIds.end(),ft->point()->id()) == pointIds.end())
                                {
                                        Vec2d pIcs = frameCur->world2image(ft->point()->position());
                                        if (    border < pIcs.x() && pIcs.x() < f->width() - border && \
                                                border < pIcs.y() && pIcs.y() < f->height() - border )
                                        {
                                                candidates.push_back(ft);
                                                pointIds.insert(ft->point()->id());
                                        }
                                        
                                }
                        }
                        
                        
                }
                return candidates;
        }

        
}