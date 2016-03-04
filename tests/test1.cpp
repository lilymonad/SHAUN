#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <map>
#include "parser.hpp"
#include "printer.hpp"

int main(void)
{
    try
    {
        shaun::parser p;
        std::ifstream file("test.shaun");
        std::ofstream file2("lel.shaun");
        std::stringstream buffer; 
        buffer << file.rdbuf();

        shaun::object obj = p.parse(buffer.str());

        shaun::printer pr(&file2);
        pr.visit(obj);
    }
    catch (shaun::parse_error e)
    {
        std::cout << e << std::endl;
    }

    return 0;
}