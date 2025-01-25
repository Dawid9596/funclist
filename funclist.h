#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <functional>
#include <ranges>
#include <sstream>
#include <type_traits>

namespace flist {
// Empty list
inline auto empty = [](auto, auto a) { return a; };

// Add [x] at the beggining of [l]
inline auto cons = [](auto x, auto l) {
    return [=](auto f, auto a) { return f(x, l(f, a)); };
};

namespace detail {
// Specialisation of create_help for base condition
auto create_help(auto l) { return l; }
// Helper function to add all elements to l
auto create_help(auto l, auto element, auto... elements) {
    return create_help(cons(element, l), elements...);
}

template <typename T>
struct is_reference_wrapper : std::false_type {};

template <typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;
} // namespace detail

// Reverse order of the list
inline auto rev = [](auto l) {
    return [l](auto f, auto a) {
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
        A result = a;
        if constexpr (detail::is_reference_wrapper_v<decltype(range)>) {
            const auto &range_ref = range.get();
            for (const auto &el : range_ref | std::views::reverse) {
                result = f(el, result);
            }
        } else {
            for (const auto &el : range | std::views::reverse) {
                result = f(el, result);
            }
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

// Filter [l] to the list of [x] such that [p]([x])
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