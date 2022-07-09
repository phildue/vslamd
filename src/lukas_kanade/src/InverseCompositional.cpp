#include <algorithm>
#include <execution>

#include "utils/utils.h"
#include "core/core.h"

#include "InverseCompositional.h"
namespace pd::vslam::lukas_kanade{

    InverseCompositional::InverseCompositional (const Image& templ, const MatXd& dTx, const MatXd& dTy, const Image& image,
     std::shared_ptr<Warp> w0,
     least_squares::Loss::ShPtr l,
     double minGradient ,
     std::shared_ptr<const least_squares::Prior> prior)
    : least_squares::Problem(w0->nParameters())
    , _T(templ)
    , _I(image)
    , _w(w0)
    , _loss(l)
    , _prior(prior)
    , _J(Eigen::MatrixXd::Zero(_T.rows() *_T.cols(),w0->nParameters()))
    {
        //TODO this could come from some external feature selector
        //TODO move dTx, dTy computation outside
        std::vector<Eigen::Vector2i> interestPoints;
        interestPoints.reserve(_T.rows() *_T.cols());
        for (int32_t v = 0; v < _T.rows(); v++)
        {
            for (int32_t u = 0; u < _T.cols(); u++)
            {
                if( std::sqrt(dTx(v,u)*dTx(v,u)+dTy(v,u)*dTy(v,u)) >= minGradient)
                {
                    interestPoints.emplace_back(u,v);
                }
                    
            }
        }
        Eigen::MatrixXd steepestDescent = Eigen::MatrixXd::Zero(_T.rows(),_T.cols());
        size_t idx = 0U;
        std::for_each(interestPoints.begin(),interestPoints.end(),[&](auto kp)
            {
                const Eigen::MatrixXd Jw = _w->J(kp.x(),kp.y());
                _J.row(idx) = Jw.row(0) * dTx(kp.y(), kp.x()) + Jw.row(1) * dTy(kp.y(),kp.x());
                const double Jnorm = _J.row(kp.y() * _T.cols() + kp.x()).norm();
                steepestDescent(kp.y(),kp.x()) = std::isfinite(Jnorm) ? Jnorm : 0.0;
                if (std::isfinite(Jnorm))
                {
                    _interestPoints.push_back({idx++,kp});
                }
            }
        );
        _J.conservativeResize(idx,Eigen::NoChange);
        LOG_IMG("SteepestDescent") << steepestDescent;
    }
    
    InverseCompositional::InverseCompositional (const Image& templ, const MatXd& dTx, const MatXd& dTy, const Image& image,
     std::shared_ptr<Warp> w0,
     const std::vector<Eigen::Vector2i>& interestPoints,
     least_squares::Loss::ShPtr l,
     std::shared_ptr<const least_squares::Prior> prior)
    : least_squares::Problem(w0->nParameters())
    , _T(templ)
    , _I(image)
    , _w(w0)
    , _loss(l)
    , _prior(prior)
    , _J(Eigen::MatrixXd::Zero(_T.rows() *_T.cols(),w0->nParameters()))
    , _interestPoints(interestPoints.size())
    {
        Eigen::MatrixXd steepestDescent = Eigen::MatrixXd::Zero(_T.rows(),_T.cols());
        std::atomic<size_t> idx = 0U;
        std::for_each(interestPoints.begin(),interestPoints.end(),[&](auto kp)
            {
                const auto Jw = _w->J(kp.x(),kp.y());
                const auto Jwi = Jw.row(0) * dTx(kp.y(), kp.x()) + Jw.row(1) * dTy(kp.y(),kp.x());
                const double Jwin = Jwi.norm();
                if (std::isfinite(Jwin))
                {
                    _J.row(idx) = Jwi;
                    steepestDescent(kp.y(),kp.x()) = Jwin;
                    _interestPoints[idx] = {idx,kp};
                    idx++;
                }
            }
        );
        _J.conservativeResize(idx,Eigen::NoChange);
        _interestPoints.resize(idx);

        LOG_IMG("SteepestDescent") << steepestDescent;
    }
    InverseCompositional::InverseCompositional (const Image& templ, const Image& image,std::shared_ptr<Warp> w0, std::shared_ptr<least_squares::Loss> l, double minGradient, std::shared_ptr<const least_squares::Prior> prior)
    : InverseCompositional (templ, algorithm::gradX(templ).cast<double>(), algorithm::gradY(templ).cast<double>(), image, w0, l, minGradient,prior){}



    void InverseCompositional::updateX(const Eigen::VectorXd& dx)
    {
        _w->updateCompositional(-dx);
    }
    least_squares::NormalEquations::ConstShPtr InverseCompositional::computeNormalEquations() 
    {
        Image IWxp = Image::Zero(_I.rows(),_I.cols());
        MatXd R = MatXd::Zero(_I.rows(),_I.cols());
        MatXd W = MatXd::Zero(_I.rows(),_I.cols());
        VecXd r = VecXd::Zero(_interestPoints.size());
        VecXd w = VecXd::Zero(_interestPoints.size());

        std::for_each(_interestPoints.begin(),_interestPoints.end(),
        [&](auto kp) {
                Eigen::Vector2d uvI = _w->apply(kp.pos.x(),kp.pos.y());
                const bool visible = 1 < uvI.x() && uvI.x() < _I.cols() - 1 && 1 < uvI.y() && uvI.y() < _I.rows() - 1 && std::isfinite(uvI.x());
                if (visible){
                    //IWxp(kp.pos.y(),kp.pos.x()) = algorithm::bilinearInterpolation(_I,uvI.x(),uvI.y());
                    IWxp(kp.pos.y(),kp.pos.x()) = _I((int)  std::round(uvI.y()),(int) std::round(uvI.x()));
                    R(kp.pos.y(),kp.pos.x()) = (double)IWxp(kp.pos.y(),kp.pos.x()) - (double)_T(kp.pos.y(),kp.pos.x());
                    W(kp.pos.y(),kp.pos.x()) = 1.0;
                    r(kp.idx) = R(kp.pos.y(),kp.pos.x());
                    w(kp.idx) = W(kp.pos.y(),kp.pos.x());
                }
            }
        );
        
        if(_loss)
        {  
            auto s = _loss->computeScale(r);
            std::for_each(_interestPoints.begin(),_interestPoints.end(),
            [&](auto kp) 
                {
                    if(w(kp.idx) > 0.0){
                        W(kp.pos.y(),kp.pos.x()) = _loss->computeWeight( (r(kp.idx)- s.offset) / s.scale);
                        w(kp.idx) = W(kp.pos.y(),kp.pos.x());
                    }
                    
                }
            );
            
        }
        auto ne = std::make_shared<least_squares::NormalEquations>(_J,r,w);
        if (ne->nConstraints() > 1)
        {
            ne->A().noalias() = ne->A() / (double)ne->nConstraints();
            ne->b().noalias() = ne->b() / (double)ne->nConstraints();
            ne->chi2() = ne->chi2() / (double)ne->nConstraints();
        }
       
        if (_prior){ _prior->apply(ne,_w->x()); }

        LOG_IMG("ImageWarped") << IWxp;
        LOG_IMG("Residual") << R;
        LOG_IMG("Weights") << W;

        return ne;


    }

}