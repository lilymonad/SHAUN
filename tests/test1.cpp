#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <map>
#include <SHAUN/parser.hpp>
#include <SHAUN/printer.hpp>

int main(void)
{
    try
    {
        shaun::parser p;
        std::ifstream file("../resources/test.shaun");
        std::stringstream buffer; 
        buffer << file.rdbuf();

        shaun::object obj = p.parse(buffer.str());

        shaun::printer pr(&std::cout);
        pr.visit(obj);
    }
    catch (shaun::parse_error e)
    {
        std::cout << e << std::endl;
    }

    return 0;
}
