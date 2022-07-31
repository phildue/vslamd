//
// Created by phil on 30.06.21.
//

#include "Feature2D.h"

namespace pd::vslam{

        std::uint64_t Feature2D::_idCtr = 0U;

        Feature2D::Feature2D(const Eigen::Vector2d& position, std::shared_ptr<FrameRgb> frame,std::shared_ptr<Point3D> p3d)
        : _position(position)
        , _frame(frame)
        , _point(p3d)
        , _id(_idCtr++)
        {

        }

    }

