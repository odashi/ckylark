#ifndef CKYLARK_BERKELEY_SIGNATURE_ESTIMATOR_H_
#define CKYLARK_BERKELEY_SIGNATURE_ESTIMATOR_H_

#include <ckylark/SignatureEstimator.h>

#include <ckylark/Dictionary.h>

namespace Ckylark {

class BerkeleySignatureEstimator : public SignatureEstimator {

public:
    enum Language {
        Other = 0,
        English = 1,
    }; // enum Language

private:
    BerkeleySignatureEstimator(const BerkeleySignatureEstimator &) = delete;
    BerkeleySignatureEstimator & operator =(const BerkeleySignatureEstimator &) = delete;

public:
    BerkeleySignatureEstimator(Language lang, const Dictionary & known_words);

    std::string getSignature(const std::vector<std::string> & sentence, size_t location);

private:
    Language lang_;
    const Dictionary & known_words_;

    // get the signature of any other latin-alphabet language's word
    std::string getOtherSignature(const std::vector<std::string> & sentence, size_t location);
    
    // get the signature of English word
    std::string getEnglishSignature(const std::vector<std::string> & sentence, size_t location);

}; // class BerkeleySignatureEstimator

} // namespace Ckylark

#endif // CKYLARK_BERKELEY_SIGNATURE_ESTIMATOR_H_

