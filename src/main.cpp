#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <map>
#include "parser.hpp"
#include "sweeper.hpp"

int main(void)
{
    try
    {
        shaun::parser p;
        std::ifstream file("test.shaun");
        std::stringstream buffer; 
        buffer << file.rdbuf();

        shaun::object obj = p.parse(buffer.str());
        shaun::sweeper swp(obj);

        try
        {
            for (int i = 0; i < 2; ++i)
            {
                shaun::number yep = swp.get("planets")[i]("physicalProperties:color")[1].value<shaun::number>();

                std::cout << (double)(yep) << std::endl;
            }
        }
        catch (std::string str)
        {
            std::cout << str << std::endl;
        }

        std::cout << "parsing successful !\n";
    }
    catch (shaun::parse_error e)
    {
        std::cout << e << std::endl;
    }

    return 0;
}