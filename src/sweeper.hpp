#ifndef VISITOR_HPP
#define VISITOR_HPP

#include "shaun.hpp"
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

    const sweeper& get(const std::string& path) const;
    const sweeper& operator[](size_t i) const;
    const sweeper& operator()(const std::string& path) const;

private:
    shaun * compute_path(const std::string& path) const;

    std::string current_name_;
    shaun * current_;
    shaun * root_;
    
    std::unique_ptr<sweeper> next_;
};

} // namespace

#endif