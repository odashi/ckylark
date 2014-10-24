#ifndef CKYLARK_STREAM_FACTORY_H_
#define CKYLARK_STREAM_FACTORY_H_

#include "Stream.h"

#include <memory>
#include <string>

namespace Ckylark {

// this class provides the switching/generating methods for each iostream
class StreamFactory {

    StreamFactory() = delete;
    StreamFactory(const StreamFactory &) = delete;
    StreamFactory & operator=(const StreamFactory &) = delete;

public:
    // create input stream from specific path
    static std::shared_ptr<InputStream> getInputStream(const std::string & path);

    // create output stream from specific path
    static std::shared_ptr<OutputStream> getOutputStream(const std::string & path);

}; // class StreamFactory

} // namespace Ckylark

#endif // CKYLARK_STREAM_FACTORY_H_

