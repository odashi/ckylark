#ifndef CKYLARK_SIGNATURE_ESTIMATOR_H_
#define CKYLARK_SIGNATURE_ESTIMATOR_H_

#include <string>
#include <vector>

namespace Ckylark {

// interface for the estimator of OOV word class
class SignatureEstimator {

    SignatureEstimator(const SignatureEstimator &) = delete;
    SignatureEstimator & operator=(const SignatureEstimator &) = delete;

public:
    SignatureEstimator() {}
    virtual ~SignatureEstimator() {}

    // estimate the signature of the location-th word in the sentence
    virtual std::string getSignature(const std::vector<std::string> & sentence, size_t location) = 0;

}; // class SignatureEstimator

} // namespace Ckylark

#endif // CKYLARK_SIGNATURE_ESTIMATOR_H_

