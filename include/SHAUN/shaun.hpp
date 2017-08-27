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

    Type type() const;
    bool is_null() const;

    /**
     * returns the index of a potential child
     * node of the current node
     */
    virtual int index_of(shaun * child) const;

    /**
     * node part of the visitor pattern
     */
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
    using const_iterator = vector::const_iterator;
    
    list();
    list(const list& l);

    ~list();

    // cf. shaun::visited()
    void visited(visitor& v);

    /**
     * same as std::vector
     */
    void push_back(shaun * elem);

    /**
     * c++11 foreach loop integration
     */
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    /**
     * returns the underlying vector of the list
     */
    const vector& elements();

    shaun& operator[](size_t i);
    const shaun& operator[](size_t i) const;

    /**
     * different from operator[]
     * as it can return a casted value
     * (i.e. list.at<double>(0) can return a double from a shaun::number)
     */
    template<class C>
    C& at(size_t i);
    template<class C>
    const C& at(size_t i) const;
    
    size_t size() const;
    virtual int index_of(shaun *child) const;

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
    number();
    number(const number& num);
    number(double val, const std::string& u);

    void visited(visitor& v);

    const std::string& unit() const;
    operator double() const { return value; }
    
private:
    double value;
    std::string un;
};

class string : public shaun
{
public: 
    string();
    string(const string& str);
    string(const std::string& str);

    void visited(visitor& v);

    operator std::string() const
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

    void visited(visitor& v);

    static null Null;
};

class object : public shaun
{
public:
    using map = std::map<std::string, std::shared_ptr<shaun> >;
    using iterator = map::iterator;
    using const_iterator = map::const_iterator;

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
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    template<class C>
    C& get(const std::string& name);

    template<class C>
    const C& get(const std::string& name) const;

    Type type_of(const std::string& name) const;
    
    size_t size() const;
    virtual int index_of(shaun *child) const;

    const std::map<std::string, std::shared_ptr<shaun> >& variables() const;
private:

    shaun * get_variable(const std::string& name) const;
    std::map<std::string, std::shared_ptr<shaun> > variables_;
};

} // namespace

#endif
