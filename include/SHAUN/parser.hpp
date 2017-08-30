#ifndef PARSER_HPP
#define PARSER_HPP

#include <clocale>
#include <cctype>
#include <sstream>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <stack>
#include <utility>
#include <iostream>
#include <SHAUN/shaun.hpp>
#include <SHAUN/exception.hpp>

namespace shaun
{

class parser
{
public:
    using char_type = char;

    parser();
    parser(const std::string& str);
    parser(const std::istringstream& iss);

    ~parser();

    object parse();
    object parse(const std::string& str);
    object parse(const std::istringstream& iss);
    object parse_file(const std::string& str);

private:

    void skipws();
    void skip_comment();
    void forward();
    

    object parse_object();
    std::pair<std::string, shaun*> parse_variable();
    std::string parse_name();
    shaun* parse_value();
    string parse_string();
    number parse_number();
    boolean parse_boolean();
    list parse_list();

    template<typename T> int signum(T val);
    
    std::istringstream iss_;
    size_t line_;
    size_t column_;
};

} // namespace

#endif
