#ifndef CKYLARK_STREAM_H_
#define CKYLARK_STREAM_H_

#include <stdexcept>

namespace Ckylark {

// interface of the input/output system
class Stream {

    Stream(const Stream &) = delete;
    Stream & operator=(const Stream &) = delete;

public:
    Stream() {}
    virtual ~Stream() {}

    // read a line and return if reading is done successfully or not
    virtual bool readLine(std::string & line) = 0;

    // write a line
    virtual void writeLine(const std::string & line) = 0;

}; // class Stream

// interface of the input system
class InputStream : public Stream {

    InputStream(const InputStream &) = delete;
    InputStream & operator=(const InputStream &) = delete;

public:
    InputStream() {}
    virtual ~InputStream() {}

    virtual bool readLine(std::string & line) = 0;
    void writeLine(const std::string & line) { throw std::runtime_error("InputStream::writeLine: not supported"); }

}; // class InputStream

// interface of the output system
class OutputStream : public Stream {

    OutputStream(const OutputStream &) = delete;
    OutputStream & operator=(const OutputStream &) = delete;

public:
    OutputStream() {}
    virtual ~OutputStream() {}

    bool readLine(std::string & line) { throw std::runtime_error("OutputStream::readLine: not supported"); }
    virtual void writeLine(const std::string & line) = 0;

}; // class OutputStream

} // namespace Ckylark

#endif // CKYLARK_STREAM_H_

