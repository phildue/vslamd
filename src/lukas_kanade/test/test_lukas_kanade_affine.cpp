//
// Created by phil on 10.10.20.
//

#include <gtest/gtest.h>
#include "utils/Exceptions.h"
#include "utils/Log.h"
#include "utils/utils.h"
#include "utils/visuals.h"
#include "core/algorithm.h"
#include "core/types.h"
#include "lukas_kanade/LukasKanade.h"
#include "lukas_kanade/LukasKanadeInverseCompositional.h"

#include "solver/Loss.h"
using namespace testing;
using namespace pd;
using namespace pd::vision;


class LukasKanadeAffineTest : public TestWithParam<int>{
    public:
    Image img0,img1;
    Eigen::Matrix3d A;
    Eigen::Vector6d x;
    LukasKanadeAffineTest()
    {
        img0 = utils::loadImage(TEST_RESOURCE"/person.jpg",50,50,true);
        A = Eigen::Matrix3d::Identity();
        img1 = img0;
        algorithm::warpAffine(img0,A,img1);
        const Eigen::Matrix3d Anoisy = transforms::createdTransformMatrix2D(random::U(1,2)*random::sign(),
        random::U(1,2)*random::sign(),
        random::U(0.025*M_PI,0.05*M_PI)*random::sign());
        x(0) = Anoisy(0,0)-1;
        x(1) = Anoisy(1,0);
        x(2) = Anoisy(0,1);
        x(3) = Anoisy(1,1)-1;
        x(4) = Anoisy(0,2);
        x(5) = Anoisy(1,2);    
    }
};
TEST_P(LukasKanadeAffineTest, DISABLED_LukasKanadeAffine)
{
     auto mat0 = vis::drawMat(img0);
    auto mat1 = vis::drawMat(img1);

    Log::getImageLog("I")->append(mat0);
    Log::getImageLog("T")->append(mat1);

    auto w = std::make_shared<WarpAffine>(x,img0.cols()/2,img0.rows()/2);
    auto gn = std::make_shared<GaussNewton<LukasKanadeAffine>> (
                0.1,
                1e-3,
                100
                );
    auto lk = std::make_shared<LukasKanadeAffine> (img1,img0,w);
    
    EXPECT_GT(w->x().norm(), 0.5);

    gn->solve(lk);
    
    EXPECT_LE(w->x().norm(), 0.5);

}

TEST_P(LukasKanadeAffineTest, LukasKanadeAffineInverseCompositional)
{
     auto mat0 = vis::drawMat(img0);
    auto mat1 = vis::drawMat(img1);

    Log::getImageLog("I")->append(mat0);
    Log::getImageLog("T")->append(mat1);

    auto w = std::make_shared<WarpAffine>(x,img0.cols()/2,img0.rows()/2);
    auto gn = std::make_shared<GaussNewton<LukasKanadeInverseCompositional<WarpAffine>>> (
                0.1,
                1e-3,
                100);
    auto lk = std::make_shared<LukasKanadeInverseCompositional<WarpAffine>> (img1,img0,w);
    
    EXPECT_GT(w->x().norm(), 0.5);

    gn->solve(lk);
    
    EXPECT_LE(w->x().norm(), 0.5);

}
INSTANTIATE_TEST_CASE_P(Instantiation, LukasKanadeAffineTest, ::testing::Range(1, 11));