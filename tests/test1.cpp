#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <functional>

#include <SHAUN/parser.hpp>
#include <SHAUN/printer.hpp>

using namespace std;

bool parsing_ok()
{
    try
    {
        // Parse file
        ifstream file("../resources/test.shaun");
        shaun::object obj1 = shaun::parse_file("../resources/test.shaun");

        // Generate shaun file
        stringstream output1;
        shaun::printer pr1(&output1);
        pr1.visit(obj1);

        // Parse generated file
        shaun::object obj2 = shaun::parse(output1.str());
        // Regenerate shaun file
        stringstream output2;
        shaun::printer pr2(&output2);
        pr2.visit(obj2);

        // Compare both files
        return output1.str() == output2.str();
    }
    catch (shaun::parse_error e)
    {
        std::cout << e << std::endl;
        return false;
    }
}

bool object_primitive_attributes()
{
    shaun::object o;
    o.add("x", 10);
    o.add("y", 10.0);
    o.add("b", true);
    o.add("s", "hello"s);

    ofstream file("../resources/test2_out.shaun");
    shaun::printer pr(&file);
    pr.visit(o);
    file.close();

    try
    {
        shaun::object o = shaun::parse_file("../resources/test2_out.shaun");
        return o.get<shaun::string>("s") == "hello";
    }
    catch (shaun::parse_error e)
    {
        std::cout << e << std::endl;
        return false;
    }
}

bool get_with_default()
{
  shaun::object o;
  o.add("x", 10);
  return o.get_with_default("default"s, "monattribut") == "default" && (double)o.get_with_default(0, "x") == 10;
}

#define MAKE_TEST(name) (UnitTest(#name, name))

class UnitTest
{
public:
    UnitTest(const string &name, const function<bool(void)> &test) : _name{name}, _test{test} {}
    const string &getName() const {return _name;}
    const function<bool(void)> &getTest() const {return _test;}
private:
    string _name;
    function<bool(void)> _test;
};

int main(void)
{
    const vector<UnitTest> unitTests = {
        MAKE_TEST(parsing_ok),
        MAKE_TEST(object_primitive_attributes),
        MAKE_TEST(get_with_default)
    };

    int failedNum = 0;
    for (const auto &test : unitTests)
    {
        const bool result = test.getTest()();
        cout << "Test " << test.getName() << (result?" successful":" failed") << endl;
        failedNum += result?0:1;
    }

    cout << "-----------------------" << endl;
    if (failedNum)
    {
        cout << failedNum << " tests failed" << endl;
    }
    else
    {
        cout << "All tests successful" << endl;
    }
}
