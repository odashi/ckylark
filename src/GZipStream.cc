#include "GZipStream.h"

using namespace std;

namespace Ckylark {

GZipInputStream::GZipInputStream(const std::string & path) {
    // open target
    ifs_.reset(new ifstream(path, ios::in | ios::binary));
    if (!ifs_->is_open()) {
        throw runtime_error("GZipInputStream::GZipInputStream: cannot open file: " + path);
    }

    // make gzip filtered stream
    buf_gzip_.reset(new boost::iostreams::filtering_streambuf<boost::iostreams::input>());
    buf_gzip_->push(boost::iostreams::gzip_decompressor());
    buf_gzip_->push(*ifs_);
    ifs_filtered_.reset(new istream(&*buf_gzip_));
}

bool GZipInputStream::readLine(string & line) {
    bool ret = !!getline(*ifs_filtered_, line);
    return ret;
}

} // namespace Ckylark

