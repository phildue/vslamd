#include <execution>
#include <vector>

#include "utils/utils.h"
#include "core/core.h"

namespace pd::vslam::lukas_kanade{

    template<typename Warp>
    InverseCompositionalStacked<Warp>::InverseCompositionalStacked ( const std::vector<std::shared_ptr<InverseCompositional<Warp>>>& frames)
    : least_squares::Problem<SE3d::DoF>()
    , _frames(frames)
    { }

    template<typename Warp>
    void InverseCompositionalStacked<Warp>::updateX(const Eigen::Matrix<double,Warp::nParameters,1>& dx)
    {
        std::for_each(_frames.begin(),_frames.end(),[&dx](auto f){f->updateX(dx);});
    }
    template<typename Warp>
    void InverseCompositionalStacked<Warp>::setX(const Eigen::Matrix<double,Warp::nParameters,1>& x)
    {
        std::for_each(_frames.begin(),_frames.end(),[&x](auto f){f->setX(x);});
    }
    template<typename Warp>
    typename least_squares::NormalEquations<Warp::nParameters>::ConstShPtr InverseCompositionalStacked<Warp>::computeNormalEquations() 
    {
        std::vector<typename least_squares::NormalEquations<Warp::nParameters>::ConstShPtr> nes(_frames.size());
        std::transform(std::execution::par_unseq,_frames.begin(),_frames.end(),nes.begin(),[&](auto f){return f->computeNormalEquations();});
        auto ne = std::make_shared<least_squares::NormalEquations<Warp::nParameters>>();
        std::for_each(nes.begin(),nes.end(),[&](auto n){ne->combine(*n);});
        return ne;
    }
   
}