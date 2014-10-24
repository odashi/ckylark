#ifndef CKYLARK_GZIP_STREAM_H_
#define CKYLARK_GZIP_STREAM_H_

#include "Stream.h"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <fstream>
#include <memory>

namespace Ckylark {

// GZIP comressed text file reader
class GZipInputStream : public InputStream {

    GZipInputStream(const GZipInputStream &) = delete;
    GZipInputStream & operator=(const GZipInputStream &) = delete;

public:
    GZipInputStream(const std::string & path);

    bool readLine(std::string & line);

private:
    std::unique_ptr<std::ifstream> ifs_;
    std::unique_ptr<std::istream> ifs_filtered_;
    std::unique_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::input> > buf_gzip_;

}; // class GZipInputStream

} // namespace Ckylark

#endif // CKYLARK_GZIP_STREAM_H_

