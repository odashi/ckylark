#include <ckylark/StdStream.h>

#include <iostream>

using namespace std;

namespace Ckylark {

StdInputStream::StdInputStream() {}

bool StdInputStream::readLine(string & line) {
    bool ret = !!getline(cin, line);
    return ret;
}

StdOutputStream::StdOutputStream() {}

void StdOutputStream::writeLine(const string & line) {
    cout << line << endl;
}

} // namespace Ckylark

