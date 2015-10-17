#include "shaun.hpp"
#include <utility>
#include <sstream>
#include <iostream>

// #define VISIT_FUN(x) void x::visited(visitor * visitor) { visitor->visit_ ## x () }
#define OBJ_GET(x) template<>                              \
    x& object::get<x> (const std::string& name)            \
    {                                                      \
        if (get_variable(name)->type() == Type::x)         \
            return *(static_cast<x*>(get_variable(name))); \
                                                           \
        throw std::string("type missmatch");               \
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

bool shaun::is_null()
{
    return type_ == Type::null;
}

Type type()
{
    return type_;
}

/*****************************
 *
 *     object functions
 *
 *****************************/

// VISIT_FUN(object);

object::object()
{
  type_ = Type::object;
}

object::object(const object& obj) : type_(Type::object), variables_(obj.variables_)
{
    type_ = Type::object;
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

/*****************************
 *
 *     list functions
 *
 *****************************/

// VISIT_FUN(list);

list::list()
{
    type_ = Type::list;
}

list::list(const list& l) : elements_(l.elements_)
{
    type_ = Type::list;
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
            throw "type missmatch";\
\
        return *static_cast<TYPE*>(ptr);\
    }

LIST_AT(list)
LIST_AT(object)
LIST_AT(number)
LIST_AT(boolean)
LIST_AT(string)

/*****************************
 *
 *     boolean functions
 *
 *****************************/

// VISIT_FUN(boolean);

boolean::boolean() : value(false)
{
    type_ = Type::boolean;
}

boolean::boolean(const boolean& b)
{
    value = b.value;
    type_ = Type::boolean;
}

boolean::boolean(bool yes) : value(yes)
{
    type_ = Type::boolean;
}

/*****************************
 *
 *     number functions
 *
 *****************************/

// VISIT_FUN(number);

number::number() : value(0), un(Unit::none)
{
    type_ = Type::number;
}

number::number(const number& num)
{
    value = num.value;
    un    = num.un;
    type_ = Type::number;
}

number::number(double val, Unit u) : value(val), un(u)
{
    type_ = Type::number;
}

number::Unit number::unit()
{
    return un;
}

/*****************************
 *
 *     string functions
 *
 *****************************/

// VISIT_FUN(string);

string::string()
{
    type_ = Type::string;
}

string::string(const string& str)
{
    value = str.value;
    type_ = Type::string;
}

string::string(const std::string& str)
{
    value = str;
    type_ = Type::string;  
}
/*****************************
 *
 *     null functions
 *
 *****************************/
 
null::null()
{
    type_ = Type::null;
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