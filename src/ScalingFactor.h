#ifndef CKYLARK_SCALING_FACTOR_H_
#define CKYLARK_SCALING_FACTOR_H_

#include "Grammar.h"

namespace Ckylark {

class ScalingFactor {

    ScalingFactor(const ScalingFactor &) = delete;
    ScalingFactor & operator=(const ScalingFactor &) = delete;

public:
    ScalingFactor() {}
    virtual ~ScalingFactor() {}

    virtual double calculate(const Grammar & grammar) = 0;

}; // class ScalingFactor

} // namespace Ckylark

#endif // CKYLARK_SCALING_FACTOR_H_

