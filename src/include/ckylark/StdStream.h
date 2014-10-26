#ifndef CKYLARK_STD_STREAM_H_
#define CKYLARK_STD_STREAM_H_

#include <ckylark/Stream.h>

namespace Ckylark {

// stream class for stdin
class StdInputStream : public InputStream {

    StdInputStream(const StdInputStream &) = delete;
    StdInputStream & operator=(const StdInputStream &) = delete;

public:
    StdInputStream();

    bool readLine(std::string & line);

}; // class StdInputStream

// stream class for stdout
class StdOutputStream : public OutputStream {

    StdOutputStream(const StdOutputStream &) = delete;
    StdOutputStream & operator=(const StdOutputStream &) = delete;

public:
    StdOutputStream();

    void writeLine(const std::string & line);

}; // class StdOutputStream

} // namespace Ckylark

#endif // CKYLARK_STD_STREAM_H_

