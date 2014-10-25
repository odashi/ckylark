#ifndef CKYLARK_TEXT_STREAM_H_
#define CKYLARK_TEXT_STREAM_H_

#include <ckylark/Stream.h>

#include <fstream>

namespace Ckylark {

// text file reader
class TextInputStream : public InputStream {

    TextInputStream(const TextInputStream &) = delete;
    TextInputStream & operator=(const TextInputStream &) = delete;

public:
    TextInputStream(const std::string & path);

    bool readLine(std::string & line);

private:
    std::ifstream ifs_;

}; // class TextInputStream

// text file writer
class TextOutputStream : public OutputStream {

    TextOutputStream(const TextOutputStream &) = delete;
    TextOutputStream & operator=(const TextOutputStream &) = delete;

public:
    TextOutputStream(const std::string & path);

    void writeLine(const std::string & line);

private:
    std::ofstream ofs_;

}; // class TextOutputStream

} // namespace Ckylark

#endif // CKYLARK_TEXT_STREAM_H_

