#include "shaun.hpp"
#include <utility>
#include <sstream>
#include <iostream>
#include "visitor.hpp"
#include "exception.hpp"

#define TYPE_FUN(TYPE) Type TYPE::type() { return Type::TYPE; }
#define VISIT_FUN(x) void x::visited(visitor& v) { v.visit_ ## x (*this); }
#define OBJ_GET(x) template<>                              \
    x& object::get<x> (const std::string& name)            \
    {                                                      \
        if (get_variable(name)->type() == Type::x)         \
            return *(static_cast<x*>(get_variable(name))); \
                                                           \
        throw type_error(Type::x, get_variable(name)->type(), name);               \
    }

#define OBJ_ADD(x) template<>                                               \
    void object::add<x> (const std::string& name, x * ptr)                  \
    {                                                                       \
        std::shared_ptr<shaun> to_add(ptr);                                 \
        variables_.insert(std::make_pair(name, to_add));                    \
    }                                                                       \
                                                                            \
    template<>                                                              \
    void object::add<x> (std::pair<std::string, x *> pair)                  \
    {                                                                       \
        std::shared_ptr<shaun> to_add(pair.second);                         \
        variables_.insert(std::make_pair(pair.first, to_add));              \
    }



namespace shaun
{

std::ostream& operator<<(std::ostream& out, const exception& err)
{
    out << err.to_string();

    return out;
}

bool shaun::is_null()
{
    return false;
}

/*****************************
 *
 *     object functions
 *
 *****************************/

TYPE_FUN(object)
VISIT_FUN(object);

object::object()
{
}

object::object(const object& obj) : variables_(obj.variables_)
{
}

object::~object()
{
}

object& object::operator=(const object& org)
{
    variables_ = org.variables_;
    return *this;
}

shaun * object::get_variable(const std::string& name)
{
    try
    {
        return variables_.at(name).get();
    }
    catch (...)
    {
        return &null::Null;
    }
}

OBJ_GET(number)
OBJ_GET(object)
OBJ_GET(boolean)
OBJ_GET(string)
OBJ_GET(list)

OBJ_ADD(number)
OBJ_ADD(object)
OBJ_ADD(boolean)
OBJ_ADD(string)
OBJ_ADD(list)

template<> void object::add<null> (const std::string& name, null * ptr) { }
template<> void object::add<null> (std::pair<std::string, null *> pair) { }

template<>
void object::add<shaun> (const std::string& name, shaun * ptr)
{
    switch (ptr->type())
    {
        case Type::number: add<number> (name, static_cast<number*>(ptr)); break;
        case Type::object: add<object> (name, static_cast<object*>(ptr)); break;
        case Type::boolean: add<boolean> (name, static_cast<boolean*>(ptr)); break;
        case Type::string: add<string> (name, static_cast<string*>(ptr)); break;
        case Type::list: add<list> (name, static_cast<list*>(ptr)); break;
        case Type::null:
        default: break;
    }
}

template<>
void object::add<shaun> (std::pair<std::string, shaun *> pair)
{
    add<shaun> (pair.first, pair.second);
}

Type object::type_of(const std::string& name)
{
    return get_variable(name)->type();
}

size_t object::size() const
{
  return variables_.size();
}

const std::map<std::string, std::shared_ptr<shaun> >& object::variables() const
{
    return variables_;
}

/*****************************
 *
 *     list functions
 *
 *****************************/

TYPE_FUN(list)
VISIT_FUN(list);

list::list()
{
}

list::list(const list& l) : elements_(l.elements_)
{
}

list::~list()
{
}

void list::push_back(shaun * elem)
{
    std::shared_ptr<shaun> ptr(elem);
    elements_.push_back(ptr);
}

const std::vector<std::shared_ptr<shaun> >& list::elements()
{
    return elements_;
}

shaun * list::operator[](size_t i)
{
    return elements_[i].get();
}

#define LIST_AT(TYPE) template<>\
    TYPE& list::at(size_t i)\
    {\
        shaun * ptr = elements_[i].get();\
        if (ptr->type() != Type::TYPE)\
            throw type_error(Type::TYPE, ptr->type(), "list element");\
\
        return *static_cast<TYPE*>(ptr);\
    }

LIST_AT(list)
LIST_AT(object)
LIST_AT(number)
LIST_AT(boolean)
LIST_AT(string)

size_t list::size() const
{
  return elements_.size();
}

/*****************************
 *
 *     boolean functions
 *
 *****************************/

TYPE_FUN(boolean)
VISIT_FUN(boolean);

boolean::boolean() : value(false)
{
}

boolean::boolean(const boolean& b)
{
    value = b.value;
}

boolean::boolean(bool yes) : value(yes)
{
}

/*****************************
 *
 *     number functions
 *
 *****************************/

TYPE_FUN(number)
VISIT_FUN(number);

number::number() : value(0), un(Unit::none)
{
}

number::number(const number& num)
{
    value = num.value;
    un    = num.un;
}

number::number(double val, Unit u) : value(val), un(u)
{
}

number::Unit number::unit()
{
    return un;
}

number::Unit::Unit() : type(Type::None), name("") { }

number::Unit::Unit(const std::string& n) : name(n)
{
    if (name == "rad")
    {
        type = Type::Rad;
    }
    else if (name == "deg")
    {
        type = Type::Deg;
    }
    else if (name == "")
    {
        type = Type::None;
    }
    else
    {
        type = Type::Custom;
    }
}

number::Unit const number::Unit::rad  = number::Unit("rad");
number::Unit const number::Unit::deg  = number::Unit("deg");
number::Unit const number::Unit::none = number::Unit();

/*****************************
 *
 *     string functions
 *
 *****************************/

TYPE_FUN(string)
VISIT_FUN(string);

string::string()
{
}

string::string(const string& str)
{
    value = str.value;
}

string::string(const std::string& str)
{
    value = str;
}
/*****************************
 *
 *     null functions
 *
 *****************************/
 
TYPE_FUN(null)
VISIT_FUN(null)

null::null()
{
}

null null::Null;

bool null::is_null()
{
    return true;
}

std::string type_to_string(Type t)
{
    switch (t)
    {
        case Type::object: return "object";
        case Type::list: return "list";
        case Type::boolean: return "boolean";
        case Type::number: return "number";
        case Type::string: return "string";
        case Type::null: return "null";
        default: return "unknown";
    }
}

}  // namespace