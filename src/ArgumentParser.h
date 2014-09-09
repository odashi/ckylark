#ifndef AHCP_ARGUMENT_PARSER_H_
#define AHCP_ARGUMENT_PARSER_H_

#include <map>
#include <string>

namespace AHCParser {

class ArgumentParser {

public:
    enum ArgumentType {
        ARGTYPE_STRING,
        ARGTYPE_INT,
        ARGTYPE_REAL,
    }; // enum ArgumentType

    ArgumentParser(const std::string & bin_name);
    ~ArgumentParser();

    void addArgument(
        ArgumentType argtype,
        const std::string & name,
        const std::string & metavar,
        const std::string & defval,
        const std::string & help,
        bool required);

    void printUsage() const;

    bool parseArgs(int argc, char * argv[]);

    std::string getString(const std::string & name) const;
    int getInt(const std::string & name) const;
    double getReal(const std::string & name) const;

private:
    struct Entry {
        ArgumentType argtype;
        std::string value;
        std::string metavar;
        std::string defval;
        std::string help;
        bool required;
    }; // struct Entry

    bool parsed_;
    std::string bin_name_;
    std::map<std::string, Entry> args_;

}; // class ArgumentParser

} // namespace AHCParser

#endif // AHCP_ARGUMENT_PARSER_H_

