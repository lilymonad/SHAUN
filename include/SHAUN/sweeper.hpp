#ifndef VISITOR_HPP
#define VISITOR_HPP

#include <SHAUN/shaun.hpp>
#include <cstddef>

namespace shaun
{

class sweeper
{
public:
    sweeper(shaun * root);
    sweeper(const sweeper& swp);
    ~sweeper();
    
    sweeper& operator=(const sweeper& swp);

    template<class C>
    C& value() const;

    Type type() const;
    bool is_null() const;
    size_t size() const;

    sweeper& get(const std::string& path);
    sweeper& operator[](size_t i);
    sweeper& operator()(const std::string& path);

private:
    shaun * compute_path(const std::string& path);

    std::string name_;
    shaun * root_;
    
    std::unique_ptr<sweeper> next_;
};

} // namespace

#endif
