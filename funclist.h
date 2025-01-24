#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <functional>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>

namespace flist {
// Empy list
inline auto empty = [](auto, auto a) { return a; };

// Add x at the beggining of l
inline auto cons = [](auto x, auto l) {
    return [=](auto f, auto a) { return f(x, l(f, a)); };
};

namespace detail {
auto create_help(auto l) { return l; }

template <typename... Args>
auto create_help(auto l, auto element, Args... elements) {
    return create_help(cons(element, l), std::forward<Args>(elements)...);
}
} // namespace detail

// Reverse order of the list
inline auto rev = [](auto l) {
    return [l](auto f, auto a) {
        // using A = std::decay_t<decltype(a)>;
        using A = decltype(a);

        auto h = [&f](auto x, std::function<A(A)> inner) {
            std::function<A(A)> result
              = [x, inner, &f](A acc) { return inner(f(x, acc)); };
            return result;
        };

        // Identity function transformer
        std::function<A(A)> identity = [](A a) { return a; };

        return l(h, identity)(a);
    };
};

// Create a list out of [elements]
inline auto create = [](auto... elements) {
    return rev(detail::create_help(empty, elements...));
};

// Create a list out of [range]
inline auto of_range = [](auto range) {
    return [range](auto f, auto a) {
        using A = decltype(a);
        using X = std::unwrap_reference_t<decltype(range)>;
        decltype(auto) rr = X(range);
        A result = a;
        for (auto el : rr | std::views::reverse) {
            result = f(el, result);
        }
        return result;
    };
};

// Concatenate two lists
inline auto concat = [](auto l, auto k) {
    return [l, k](auto f, auto a) { return l(f, k(f, a)); };
};

// Map function [m] on elements of the list
inline auto map = [](auto m, auto l) {
    return [m, l](auto f, auto a) {
        return l([m, f](auto x, auto acc) { return f(m(x), acc); }, a);
    };
};

// Filter [l] to [x] such that [p]([x])
inline auto filter = [](auto p, auto l) {
    return [p, l](auto f, auto a) {
        return l([p, f](auto x, auto acc) { return p(x) ? f(x, acc) : acc; },
                 a);
    };
};

// Flatten a list of lists to a list
inline auto flatten = [](auto l) {
    return [l](auto f, auto a) {
        return l([f](auto sublist, auto acc) { return sublist(f, acc); }, a);
    };
};

// Represent [l] as a string
inline auto as_string = [](const auto &l) {
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    auto lrev = rev(l);
    lrev(
      [&](auto x, auto) {
          if (!first)
              oss << ";";
          first = false;
          oss << x;
          return empty;
      },
      empty);
    oss << "]";
    return oss.str();
};

} // namespace flist

#endif // FUNCLIST_H