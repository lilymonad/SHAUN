#include "shaun.hpp"
#include <utility>
#include <sstream>
#include <iostream>
#include "visitor.hpp"
#include "exception.hpp"

#define VISIT_FUN(x) void x::visited(visitor& v) { v.visit_ ## x (*this); }
#define OBJ_GET(x) template<>                                        \
    x& object::get<x> (const std::string& name)                      \
    {                                                                \
        if (get_variable(name)->type() == Type::x)                   \
            return *(static_cast<x*>(get_variable(name)));           \
                                                                     \
        throw type_error(Type::x, get_variable(name)->type(), name); \
    }                                                                \
    template<>                                                       \
    const x& object::get<x> (const std::string& name) const          \
    {                                                                \
        if (get_variable(name)->type() == Type::x)                   \
            return *(static_cast<const x*>(get_variable(name)));     \
                                                                     \
        throw type_error(Type::x, get_variable(name)->type(), name); \
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

shaun::shaun(Type t) : type_(t)
{

}

bool shaun::is_null()
{
    return type_ != Type::null;
}

Type shaun::type()
{
    return type_;
}

/*****************************
 *
 *     object functions
 *
 *****************************/

VISIT_FUN(object)

object::object() : shaun(Type::object)
{
}

object::object(const object& obj) : shaun(Type::object), variables_(obj.variables_)
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

shaun * object::get_variable(const std::string& name) const
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


object::iterator object::begin()
{
    return variables_.begin();
}

object::iterator object::end()
{
    return variables_.end();
}

template<> void object::add<null> (const std::string& name, null * ptr) { }
template<> void object::add<null> (std::pair<std::string, null *> pair) { }

template<>
void object::add<shaun> (const std::string& name, shaun * ptr)
{
    switch (ptr->type())
    {
        case Type::number:
        case Type::object:
        case Type::boolean:
        case Type::string:
        case Type::list:
            variables_.insert(std::make_pair(name, std::shared_ptr<shaun>(ptr)));
        break;

        // no pollution
        case Type::null:
        default: break;
    }
}

template<>
void object::add<shaun> (std::pair<std::string, shaun *> pair)
{
    add(pair.first, pair.second);
}

Type object::type_of(const std::string& name) const
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

VISIT_FUN(list)

list::list() : shaun(Type::list)
{
}

list::list(const list& l) : shaun(Type::list), elements_(l.elements_)
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

list::iterator list::begin()
{
    return elements_.begin();
}

list::iterator list::end()
{
    return elements_.end();
}

const std::vector<std::shared_ptr<shaun> >& list::elements()
{
    return elements_;
}

shaun& list::operator[](size_t i)
{
    return *elements_[i];
}

const shaun& list::operator[](size_t i) const
{
    return *elements_[i];
}

#define LIST_AT(TYPE) template<>\
    TYPE& list::at<TYPE>(size_t i)\
    {\
        shaun * ptr = elements_[i].get();\
        if (ptr->type() != Type::TYPE)\
            throw type_error(Type::TYPE, ptr->type(), "list element");\
\
        return *static_cast<TYPE*>(ptr);\
    }\
    template<>\
    const TYPE& list::at<TYPE>(size_t i) const\
    {\
        shaun * ptr = elements_[i].get();\
        if (ptr->type() != Type::TYPE)\
            throw type_error(Type::TYPE, ptr->type(), "list element");\
\
        return *static_cast<const TYPE*>(ptr);\
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

VISIT_FUN(boolean)

boolean::boolean() : shaun(Type::boolean), value(false)
{
}

boolean::boolean(const boolean& b) : shaun(Type::boolean), value(b.value)
{
}

boolean::boolean(bool yes) : shaun(Type::boolean), value(yes)
{
}

/*****************************
 *
 *     number functions
 *
 *****************************/

VISIT_FUN(number)

number::number() : shaun(Type::number), value(0), un(Unit::none)
{
}

number::number(const number& num) : shaun(Type::number)
{
    value = num.value;
    un    = num.un;
}

number::number(double val, Unit u) : shaun(Type::number), value(val), un(u)
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

VISIT_FUN(string)

string::string() : shaun(Type::string)
{
}

string::string(const string& str) : shaun(Type::string)
{
    value = str.value;
}

string::string(const std::string& str) : shaun(Type::string)
{
    value = str;
}

/*****************************
 *
 *     null functions
 *
 *****************************/

VISIT_FUN(null)

null::null() : shaun(Type::null)
{
}

null null::Null;

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
