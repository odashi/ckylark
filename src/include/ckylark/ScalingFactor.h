#ifndef CKYLARK_SCALING_FACTOR_H_
#define CKYLARK_SCALING_FACTOR_H_

namespace Ckylark {

class ScalingFactor {

    ScalingFactor(const ScalingFactor &) = delete;
    ScalingFactor & operator=(const ScalingFactor &) = delete;

public:
    ScalingFactor() {}
    virtual ~ScalingFactor() {}

    virtual double getLexiconScalingFactor(int word_id) const = 0;
    virtual double getGrammarScalingFactor() const = 0;

}; // class ScalingFactor

} // namespace Ckylark

#endif // CKYLARK_SCALING_FACTOR_H_

