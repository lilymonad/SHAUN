#ifndef shaun_HPP
#define shaun_HPP

#include <map>
#include <vector>
#include <memory>

namespace shaun
{
  
class visitor;

enum class Type : int { object, list, boolean, number, string, null };

std::string type_to_string(Type t);

class shaun
{
public:    
    virtual ~shaun() {}

    Type type();
    bool is_null();

    virtual void visited(visitor& v) = 0;
protected:
    shaun() = delete;
    shaun(Type t);

    Type type_;
};



class list : public shaun
{
public:
  
    using vector = std::vector<std::shared_ptr<shaun> >;
    using iterator = vector::iterator;
    
    list();
    list(const list& l);
    ~list();
    void visited(visitor& v);

    void push_back(shaun * elem);
    iterator begin();
    iterator end();
    const vector& elements();

    shaun& operator[](size_t i);
    const shaun& operator[](size_t i) const;

    template<class C>
    C& at(size_t i);

    template<class C>
    const C& at(size_t i) const;
    
    size_t size() const;

private:
    vector elements_;
};

class boolean : public shaun
{
public:
    boolean();
    boolean(const boolean& b);
    boolean(bool yes);
    void visited(visitor& v);

    explicit operator bool() const { return value; }
private:
    bool value;
};

class number : public shaun
{
public:
    class Unit
    {
    public:
        enum Type : int { Rad, Deg, None, Custom };

        Unit();

        Unit(const std::string& n);

        Type type;
        std::string name;

        static const Unit rad;
        static const Unit deg;
        static const Unit none;
    };

    number();
    number(const number& num);
    number(double val, Unit u);
    void visited(visitor& v);

    Unit unit();
    explicit operator double() const { return value; }
    
private:
    double value;
    Unit un;
};

class string : public shaun
{
public: 
    string();
    string(const string& str);
    string(const std::string& str);
    void visited(visitor& v);

    explicit operator std::string() const
    {
        return value;
    }
private:
    std::string value;
};

class null : public shaun
{
public:
    null();
    Type type();
    void visited(visitor& v);

    static null Null;
};

class object : public shaun
{
public:
    using map = std::map<std::string, std::shared_ptr<shaun> >;
    using iterator = map::iterator;

    friend class sweeper;

    object();
    object(const object& obj);
    ~object();
    void visited(visitor& v);

    object& operator=(const object& obj);

    template<class C>
    void add(std::pair<std::string, C *> pair);

    template<class C>
    void add(const std::string& name, C * ptr);

    iterator begin();
    iterator end();

    template<class C>
    C& get(const std::string& name);

    template<class C>
    const C& get(const std::string& name) const;

    Type type_of(const std::string& name) const;
    
    size_t size() const;

    const std::map<std::string, std::shared_ptr<shaun> >& variables() const;
private:

    shaun * get_variable(const std::string& name) const;
    std::map<std::string, std::shared_ptr<shaun> > variables_;
};

} // namespace

#endif
