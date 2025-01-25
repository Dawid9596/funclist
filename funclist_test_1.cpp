#include "funclist.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace
{
    template <typename... Args>
    auto create_ref(const Args&... args)
    {
        return flist::create(std::ref(args)...);
    }
}

int main()
{
    auto add = []<typename A>(auto x, A a) -> A {return x + a;};
    auto l1 = flist::cons(1, flist::cons(2.5, flist::empty));
    assert(flist::as_string(l1) == "[1;2.5]");
    assert(l1(add, 0.0) == 3.5);
    assert(l1(add, 0) == 3);
    auto l2 = flist::rev(flist::cons("char[]", std::ref(l1)));
    assert(flist::as_string(l2) == "[2.5;1;char[]]");
    auto l3 = flist::create('c', "s", std::string("string"));
    auto l4 = flist::empty;
    auto l5 = flist::concat(flist::of_range(std::array{3, 5, 7}), std::ref(l1));
    assert(flist::as_string(l5) == "[3;5;7;1;2.5]");
    assert(l5(add, 0.0) == 18.5);
    auto l6 = flist::rev(std::ref(l5));
    assert(flist::as_string(l6) == "[2.5;1;7;5;3]");

    auto ll = create_ref(l1, l2, l3, l4);
    assert(flist::as_string(flist::map(flist::as_string, flist::map(flist::rev, std::ref(ll)))) ==
        "[[2.5;1];[char[];1;2.5];[string;s;c];[]]");
    auto flat_ll = flist::flatten(std::ref(ll));
    assert(flist::as_string(flat_ll) == "[1;2.5;2.5;1;char[];c;s;string]");
    auto l7 = flist::filter([](auto x) {return !std::is_integral_v<decltype(x)>;}, std::ref(flat_ll));
    assert(flist::as_string(l7) == "[2.5;2.5;char[];s;string]");

    auto custom_l = [i = 2](auto f, auto a)
    {
        return f("This is ", f(std::ref(i), f(' ', f("custom call", a))));
    };
    assert(flist::as_string(custom_l) == "[This is ;2; ;custom call]");
    assert(flist::as_string(flist::rev(custom_l)) == "[custom call; ;2;This is ]");
    assert(flist::as_string(flist::rev(flist::rev(custom_l))) == "[This is ;2; ;custom call]");

    constexpr int N = 16;
    std::array<std::vector<int>, N> sets;
    for (int pot2 = 1; pot2 < N; pot2 *= 2)
        for (int i = 0; i < N; i++)
            if (i & pot2)
                sets[i].push_back(pot2);
    auto set_of_lists =
        [&]<std::size_t... Is>(std::index_sequence<Is...>)
        {
            return std::array{flist::of_range(std::ref(sets[Is]))...};
        } (std::make_index_sequence<N>{});
    auto list_of_lists = flist::of_range(std::ref(set_of_lists));
    auto seqN = flist::map([&](auto l) {return l(add, 0);}, flist::rev(std::ref(list_of_lists)));
    assert(seqN([](int i, int a) {assert(i == a); return a + 1;}, 0) == N);
    
    
    auto lm = flist::create(1, 2, 3);
    const int x = 2;
    assert(flist::rev(lm)(add, std::move(x)) == 8);
}
