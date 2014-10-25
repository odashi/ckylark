#ifndef CKYLARK_MAX_SCALING_FACTOR_H_
#define CKYLARK_MAX_SCALING_FACTOR_H_

#include <ckylark/ScalingFactor.h>

namespace Ckylark {

class MaxScalingFactor : public ScalingFactor {

    MaxScalingFactor(const MaxScalingFactor &) = delete;
    MaxScalingFactor & operator=(const MaxScalingFactor &) = delete;

public:
    MaxScalingFactor();
    ~MaxScalingFactor();

    double calculate(const Lexicon & lexicon, const Grammar & grammar);

}; // class MaxScalingFactor

} // namespace Ckylark

#endif // CKYLARK_MAX_SCALING_FACTOR_H_

