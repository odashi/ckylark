#include <ckylark/TextStream.h>

#include <stdexcept>

using namespace std;

namespace Ckylark {

TextInputStream::TextInputStream(const string & path)
    : ifs_(path) {

    if (!ifs_.is_open()) {
        throw runtime_error("TextInputStream::TextInputStream: cannot open file: " + path);
    }
}

bool TextInputStream::readLine(string & line) {
    bool ret = !!getline(ifs_, line);
    return ret;
}

TextOutputStream::TextOutputStream(const string & path)
    : ofs_(path) {

    if (!ofs_.is_open()) {
        throw runtime_error("TextOutputStream::TextOutputStream: cannot open file: " + path);
    }
}

void TextOutputStream::writeLine(const string & line) {
    ofs_ << line << endl;
}

} // namespace Ckylark

