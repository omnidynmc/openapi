#include <string>
#include <iostream>
#include <cstdio>
#include <streambuf>
#include <iomanip>
#include <fstream>

#include <unistd.h>

using namespace std;

// This is the streambuffer; its function is to store formatted data and send
// it to a character output when solicited (sync/overflow methods) . You do not
// instantiate it by yourself on your application; it will be automatically used
// by an actual output stream (like the TimestampLoggerStream class defined ahead)
class TimestampLoggerStreambuf : public streambuf {
protected:
  static const int bufferSize = 32;   // size of data buffer
  char buffer[bufferSize];            // data buffer

public:
    TimestampLoggerStreambuf() { setp (buffer, buffer+(bufferSize-1)); }
    virtual ~TimestampLoggerStreambuf() { sync(); }

protected:
    string LineHeader() {
        time_t secondsSinceEpoch = time(NULL);
        tm* brokenTime = localtime(&secondsSinceEpoch);
        char buf[80];
        strftime(buf, sizeof(buf), "[%d/%m/%y %T] ", brokenTime);
        return string(buf);
    }
    // flush the characters in the buffer
    int flushBuffer () {
        int num = pptr()-pbase();
        if (write(1, buffer, num) != num) {
            return EOF;
        }
        pbump(-num); // reset put pointer accordingly
        return num;
    }
    virtual int overflow ( int c = EOF ) {
        if (c != EOF) {
            *pptr() = c;    // insert character into the buffer
            pbump(1);
        }
        if (flushBuffer() == EOF)
            return EOF;
        return c;
    }
    virtual int sync() {
        int num = pptr()-pbase();
        if (num > 0)
          cout << LineHeader() << flush;

        if (flushBuffer() == EOF)
            return -1;    // ERROR
        return 0;
    }
};

// This is the output stream; its function is to format data (using mainly the <<
// operator) and send it to a streambuf to be stored and written to the output.
class TimestampLoggerStream : public ostream
{
public:
    TimestampLoggerStream() : ostream(new TimestampLoggerStreambuf()), ios(0) {}
    ~TimestampLoggerStream() { delete rdbuf(); }
};

// Demo main() function, simply write a few lines to standard output. The endl
// manipulator indicates the end of a text line and requests a flush. This will
// trigger a call to TimestampLoggerStreambuf::sync() which will output the
// timestamp header followed by the line of text.
int main(int argc, char *argv[])
{
    cout << "Application started." << endl;

    TimestampLoggerStream elog;

    elog << "abc";
    elog << "def" << endl;
    elog << "123" << endl;
    elog << boolalpha << true << endl;

   ofstream f("test.txt");
   f << "This is a test" << endl;

    return 0;
}
