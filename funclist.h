#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <functional>
#include <iterator>
#include <sstream>

namespace flist {
// Empty list
constexpr auto empty = [](auto, auto a) { return a; };

// Add [x] at the beggining of [l]
constexpr auto cons = [](auto x, auto l) {
    return [=](auto &&f, auto a) { return f(x, l(f, a)); };
};

namespace detail {
// Specialisation of create_help for the base condition
constexpr auto create_help() { return empty; }
// Function to add [element] to a list made of [elements...]
constexpr auto create_help(auto element, auto... elements) {
    return cons(element, create_help(elements...));
}
} // namespace detail

// Reverse order of the list
constexpr auto rev = [](auto l) {
    return [l](auto &&f, auto a) {
        using A = decltype(a);
        using A_to_A = std::function<A(A)>;

        auto h = [&f](auto x, A_to_A inner) {
            A_to_A result = [x, inner, &f](A acc) { return inner(f(x, acc)); };
            return result;
        };

        // Identity function transformer
        A_to_A identity = [](A a) { return a; };

        return l(h, identity)(a);
    };
};

// Create a list out of [elements]
constexpr auto create = [](auto... elements) {
    return detail::create_help(elements...);
    // Implementation offloaded to create_help for function overloads
};

// Create a list out of elements from [range_container]
constexpr auto of_range = [](auto range_container) {
    return [range_container](auto &&f, auto a) {
        const auto &range_ref
          = std::unwrap_reference_t<decltype(range_container)>(range_container);

        auto h = [&f, &a](auto &&self, auto it, auto end) {
            if (it == end)
                return a;
            return f(*it, self(self, std::next(it), end));
            // = f(x1, f(x2, ...f(xk, a)...));
        };

        return h(h, range_ref.begin(), range_ref.end());
    };
};

// Concatenate the two lists
constexpr auto concat = [](auto l, auto k) {
    return [l, k](auto &&f, auto a) { return l(f, k(f, a)); };
};

// Map function [m] onto elements of the list
constexpr auto map = [](auto m, auto l) {
    return [m, l](auto &&f, auto a) {
        return l([&m, &f](auto x, auto acc) { return f(m(x), acc); }, a);
    };
};

// Filter [l] to the list of [x] such that [p]([x])
constexpr auto filter = [](auto p, auto l) {
    return [p, l](auto &&f, auto a) {
        return l([&p, &f](auto x, auto acc) { return p(x) ? f(x, acc) : acc; },
                 a);
    };
};

// Flatten the list of lists to a list
constexpr auto flatten = [](auto l) {
    return [l](auto &&f, auto a) {
        return l([&f](auto sublist, auto acc) { return sublist(f, acc); }, a);
    };
};

// Represent [l] as a string
constexpr auto as_string = [](const auto &l) {
    std::ostringstream oss;
    oss << "[";

    bool first = true;
    auto lrev = rev(l);
    lrev(
      [&first, &oss](auto x, auto) {
          if (!first)
              oss << ";";
          first = false;
          oss << x;
          return 0;
      },
      0);

    oss << "]";

    return oss.str();
};
} // namespace flist

#endif // FUNCLIST_H
