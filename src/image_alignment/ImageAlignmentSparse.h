#ifndef DIRECT_IMAGE_IMAGE_ALIGNMENT_H__
#define DIRECT_IMAGE_IMAGE_ALIGNMENT_H__


#include <sophus/se3.hpp>
#include <ceres/ceres.h>

#include "core/Frame.h"

namespace pd{namespace vision{

    class ImageAlignmentSparse
    {
    public:
        explicit ImageAlignmentSparse(uint32_t patchSize, uint32_t levelMax, uint32_t levelMin);
        virtual void align(Frame::ShConstPtr referenceFrame, Frame::ShConstPtr targetFrame) const;

    protected:
        const int _levelMax,_levelMin,_patchSize;
    };



}}

#endif //IMAGE_ALIGNMENT_H__