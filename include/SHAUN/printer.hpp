#ifndef printer_HPP
#define printer_HPP

#include "visitor.hpp"
#include <ostream>
#include <vector>

namespace shaun
{

class printer : public visitor
{
public:
    printer(std::ostream * stream);
    ~printer();

    void visit(shaun& s);

    void visit_null(null& n);
    void visit_boolean(boolean& b);
    void visit_number(number& n);
    void visit_string(string& s);
    void visit_list(list& l);
    void visit_object(object& o);

private:
    unsigned int spaces();
    void print_spaces(long long int n);

    std::vector<unsigned int> indent_levels_;
    std::ostream * stream_;
};

} // namespace

#endif