#pragma once
#include <cstddef>
namespace pong
{
    // For use with system internals, because I'm lazy and don't want to
    // use templates
    // I think bit flags might be better if I want to have hybrid systems
    // with multiple update functions
    enum tags
    {
        null,
        indep,
        coll,
        particle,
        
    };

    // https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
    struct TagsHashClass
    {
        template <typename T>
        std::size_t operator()(T t) const noexcept 
        {
            return (int)(t);
        }
    };

}