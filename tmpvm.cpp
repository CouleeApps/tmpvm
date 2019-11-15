#include <stdio.h>
#include <typeinfo>
#include <type_traits>

struct number_base {};

template<size_t N>
struct num : number_base {
    static const size_t value = N;
    template<typename M>
    using add = num<N + M::value>;
    template<typename M>
    using sub = num<N - M::value>;
    template<typename M>
    using mult = num<N * M::value>;
    template<typename M>
    using idiv = num<N / M::value>;
    template<typename M>
    using mod = num<N % M::value>;
};

template<typename T, typename M>
using add = typename T::template add<M>;
template<typename T, typename M>
using sub = typename T::template sub<M>;
template<typename T, typename M>
using mult = typename T::template mult<M>;
template<typename T, typename M>
using idiv = typename T::template idiv<M>;
template<typename T, typename M>
using mod = typename T::template mod<M>;


struct address_base {};

template<size_t N>
struct addr : address_base {
    static const size_t value = N;
    template<size_t M>
    using plus = addr<N + M>;
};

template<typename T, size_t M>
using plus = typename T::template plus<M>;

struct char_base {};

template<char C>
struct ch : char_base {
    static const char value = C;
};

struct opcode_base {};

struct NOP : opcode_base {};
struct DUMP : opcode_base {};
struct PUSH : opcode_base {};
struct POP : opcode_base {};
struct DUP : opcode_base {};
struct ADD : opcode_base {};
struct SUB : opcode_base {};
struct MUL : opcode_base {};
struct IDIV : opcode_base {};
struct MOD : opcode_base {};
struct JMP : opcode_base {};
struct JNZ : opcode_base {};
struct CALL : opcode_base {};
struct RET : opcode_base {};
struct PRINT : opcode_base {};
struct PUTCHAR : opcode_base {};
struct HLT : opcode_base {};

using len_t = size_t;

template <typename ... rest>
struct list_t {
    static const len_t count = sizeof...(rest);
    template<typename ... o>
    using cons = list_t<o..., rest...>;
    template<typename ... o>
    using snoc = list_t<rest..., o...>;
};

template<>
struct list_t<> {
    static const len_t count = 0;
    template<typename ... o>
    using cons = list_t<o...>;
    template<typename ... o>
    using snoc = list_t<o...>;
};

template <typename i, typename ... rest>
struct list_t<i, rest...> {
    static const len_t count = sizeof...(rest) + 1;
    using head = i;
    typedef list_t<rest...> tail;
    template<typename ... o>
    using cons = list_t<o..., i, rest...>;
    template<typename ... o>
    using snoc = list_t<i, rest..., o...>;
};

template<typename head, typename tail>
using cons = typename head::template cons<tail>;
template<typename head, typename tail>
using snoc = typename head::template snoc<tail>;

template <typename stack, size_t idx>
struct at_idx {
    using value = typename at_idx<typename stack::tail, idx - 1>::value;
};

template<typename stack>
struct at_idx<stack, 0> {
    using value = typename stack::head;
};

template<typename stack, typename num>
struct at {
    using value = typename at_idx<stack, num::value>::value;
};

template<typename head, typename tail>
struct append {
    using value = typename append<typename head::template snoc<typename tail::head>, typename tail::tail>::value;
};

template<typename head>
struct append<head, list_t<>> {
    using value = head;
};

template<typename head, typename tail, typename ... tails>
struct append_lists {
    using value = typename append_lists<typename append<head, tail>::value, tails...>::value;
};

template<typename head, typename tail>
struct append_lists<head, tail> {
    using value = typename append<head, tail>::value;
};

template<typename stack, size_t i = 0>
void print_stack() {
    if constexpr (stack::count > 0) {
        printf("%zu: %d\n", i, stack::head);
        print_stack<typename stack::tail, i + 1>();
    } else {
        printf("\n");
    }
}

template<size_t n>
void print() {
    printf("%zu", n);
}

template<char c>
void put_char() {
    printf("%c", c);
}

template<typename ops, typename pc, typename stack>
int exec() {
    using op = typename at<ops, pc>::value;
    static_assert(std::is_base_of_v<opcode_base, op>, "Unknown opcode");

    if constexpr (std::is_same_v<op, NOP>) {
        return exec<ops, plus<pc, 1>, stack>();
    }
    else if constexpr (std::is_same_v<op, DUMP>) {
        print_stack<stack>();
        return exec<ops, plus<pc, 1>, stack>();
    }
    else if constexpr (std::is_same_v<op, PUSH>) {
        return exec<ops, plus<pc, 2>, cons<stack, typename at<ops, plus<pc, 1>>::value>>();
    }
    else if constexpr (std::is_same_v<op, POP>) {
        return exec<ops, plus<pc, 1>, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<op, DUP>) {
        return exec<ops, plus<pc, 1>, cons<stack, typename stack::head>>();
    }
    else if constexpr (std::is_same_v<op, ADD>) {
        return exec<ops, plus<pc, 1>, cons<typename stack::tail::tail, add<typename stack::tail::head, typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<op, SUB>) {
        return exec<ops, plus<pc, 1>, cons<typename stack::tail::tail, sub<typename stack::tail::head, typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<op, MUL>) {
        return exec<ops, plus<pc, 1>, cons<typename stack::tail::tail, mult<typename stack::tail::head, typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<op, IDIV>) {
        return exec<ops, plus<pc, 1>, cons<typename stack::tail::tail, idiv<typename stack::tail::head, typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<op, MOD>) {
        return exec<ops, plus<pc, 1>, cons<typename stack::tail::tail, mod<typename stack::tail::head, typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<op, JMP>) {
        return exec<ops, typename at<ops, plus<pc, 1>>::value, stack>();
    }
    else if constexpr (std::is_same_v<op, JNZ>) {
        return exec<ops, typename std::conditional_t<stack::head::value != 0, typename at<ops, plus<pc, 1>>::value, plus<pc, 2>>, stack>();
    }
    else if constexpr (std::is_same_v<op, CALL>) {
        return exec<ops, typename at<ops, plus<pc, 1>>::value, cons<stack, plus<pc, 2>>>();
    }
    else if constexpr (std::is_same_v<op, RET>) {
        return exec<ops, typename stack::head, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<op, PRINT>) {
        static_assert(std::is_base_of_v<number_base, typename at<ops, plus<pc, 1>>::value>, "Bad opcode argument");
        print<at<stack, typename at<ops, plus<pc, 1>>::value>::value::value>();
        return exec<ops, plus<pc, 2>, stack>();
    }
    else if constexpr (std::is_same_v<op, PUTCHAR>) {
        static_assert(std::is_base_of_v<char_base, typename at<ops, plus<pc, 1>>::value>, "Bad opcode argument");
        put_char<at<ops, plus<pc, 1>>::value::value>();
        return exec<ops, plus<pc, 2>, stack>();
    }
    else if constexpr (std::is_same_v<op, HLT>) {
        return stack::head::value;
    }
}

int main() {
    typedef list_t<
        PUTCHAR, ch<'F'>, PUTCHAR, ch<'i'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'\n'>, RET
        > fizz;
    typedef list_t<
        PUTCHAR, ch<'B'>, PUTCHAR, ch<'u'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'\n'>, RET
        > buzz;
    typedef list_t<
        PUTCHAR, ch<'F'>, PUTCHAR, ch<'i'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'B'>, PUTCHAR, ch<'u'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'z'>, PUTCHAR, ch<'\n'>, RET
        > fizzbuzz;

    static const size_t fizz_addr = 2;
    static const size_t buzz_addr = fizz_addr + fizz::count;
    static const size_t fizzbuzz_addr = buzz_addr + buzz::count;
    static const size_t main_addr = fizzbuzz_addr + fizzbuzz::count;

    typedef append_lists<
        list_t<JMP, addr<main_addr>>,
        fizz,
        buzz,
        fizzbuzz,
        list_t<
            /* 0: main_addr */
            PUSH, num<0>,
            JMP, addr<main_addr + 5>,

            /* 4: nextloop */
            POP,

            /* 5: loop */
            PUSH, num<1>, ADD,

            /* 8: if i % 15 == 0 fizzbuzz() { */
            DUP, PUSH, num<15>, MOD, JNZ, addr<main_addr + 19>, CALL, addr<fizzbuzz_addr>, POP, JMP, addr<main_addr + 48>,
            /* 19: } else { */
            POP,
            /* 20: if i % 5 == 0 buzz() { */
            DUP, PUSH, num<5>, MOD, JNZ, addr<main_addr + 31>, CALL, addr<buzz_addr>, POP, JMP, addr<main_addr + 48>,
            /* 31: } else { */
            POP,
            /* 32: if i % 3 == 0 fizz() { */
            DUP, PUSH, num<3>, MOD, JNZ, addr<main_addr + 43>, CALL, addr<fizz_addr>, POP, JMP, addr<main_addr + 48>,
            /* 43: } else { */
            POP,
            /* 44: print i */
            PRINT, num<0>, PUTCHAR, ch<'\n'>,
            /* 48: } */

            /* 48: if i != 25 goto nextloop */
            DUP, PUSH, num<25>, SUB, JNZ, addr<main_addr + 4>, POP,
            /* exit i */
            HLT
        >
    >::value opcodes;
    typedef list_t<num<0>> stack;
    printf("Returned: %d\n", exec<opcodes, addr<0>, stack>());
    return 0;
}