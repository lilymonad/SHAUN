#ifndef visitor_HPP
#define visitor_HPP

#include <SHAUN/shaun.hpp>

namespace shaun
{

    class visitor
    {
    public:
        virtual ~visitor() {}

        virtual void visit(shaun& s) { s.visited(*this); }

        virtual void visit_null(null& n) = 0;
        virtual void visit_boolean(boolean& b) = 0;
        virtual void visit_number(number& n) = 0;
        virtual void visit_string(string& s) = 0;
        virtual void visit_list(list& l) = 0;
        virtual void visit_object(object& o) = 0;
    };

} // namespace

#endif
