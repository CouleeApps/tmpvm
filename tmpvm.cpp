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

struct address_base {};

template<size_t N>
struct addr : address_base {
    static const size_t value = N;
    template<size_t M>
    using plus = addr<N + M>;
};

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
struct MAX : opcode_base {};

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

template <size_t idx, typename stack>
struct at {
    using value = typename at<idx - 1, typename stack::tail>::value;
};

template<typename stack>
struct at<0, stack> {
    using value = typename stack::head;
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

template<typename ops, typename pc, typename stack>
int exec() {
    static_assert(std::is_base_of_v<opcode_base, typename at<pc::value, ops>::value>, "Unknown opcode");

    if constexpr (std::is_same_v<typename at<pc::value, ops>::value, NOP>) {
        return exec<ops, typename pc::template plus<1>, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, DUMP>) {
        print_stack<stack>();
        return exec<ops, typename pc::template plus<1>, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, PUSH>) {
        return exec<ops, typename pc::template plus<2>, typename stack::template cons<typename at<pc::template plus<1>::value, ops>::value>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, POP>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, DUP>) {
        return exec<ops, typename pc::template plus<1>, typename stack::template cons<typename stack::head>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, ADD>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail::tail::template cons<typename stack::tail::head::template add<typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, SUB>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail::tail::template cons<typename stack::tail::head::template sub<typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, MUL>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail::tail::template cons<typename stack::tail::head::template mul<typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, IDIV>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail::tail::template cons<typename stack::tail::head::template idiv<typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, MOD>) {
        return exec<ops, typename pc::template plus<1>, typename stack::tail::tail::template cons<typename stack::tail::head::template mod<typename stack::head>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, JMP>) {
        return exec<ops, typename at<pc::template plus<1>::value, ops>::value, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, JNZ>) {
        return exec<ops, typename std::conditional_t<stack::head::value != 0, typename at<pc::template plus<1>::value, ops>::value, typename pc::template plus<2>>, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, CALL>) {
        return exec<ops, typename at<pc::template plus<1>::value, ops>::value, typename stack::template cons<typename pc::template plus<2>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, RET>) {
        return exec<ops, typename stack::head, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, PRINT>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc::template plus<1>::value, ops>::value>, "Bad opcode argument");
        printf("%zu\n", at<at<pc::template plus<1>::value, ops>::value::value, stack>::value::value);
        return exec<ops, typename pc::template plus<2>, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, PUTCHAR>) {
        static_assert(std::is_base_of_v<char_base, typename at<pc::template plus<1>::value, ops>::value>, "Bad opcode argument");
        printf("%c", at<pc::template plus<1>::value, ops>::value::value);
        return exec<ops, typename pc::template plus<2>, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc::value, ops>::value, HLT>) {
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
            DUP, PUSH, num<15>, MOD, JNZ, addr<main_addr + 19>, CALL, addr<fizzbuzz_addr>, POP, JMP, addr<main_addr + 46>,
            /* 19: } else { */
            POP,
            /* 20: if i % 5 == 0 buzz() { */
            DUP, PUSH, num<5>, MOD, JNZ, addr<main_addr + 31>, CALL, addr<buzz_addr>, POP, JMP, addr<main_addr + 46>,
            /* 31: } else { */
            POP,
            /* 32: if i % 3 == 0 fizz() { */
            DUP, PUSH, num<3>, MOD, JNZ, addr<main_addr + 43>, CALL, addr<fizz_addr>, POP, JMP, addr<main_addr + 46>,
            /* 43: } else { */
            POP,
            /* 44: print i */
            PRINT, num<0>,
            /* 46: } */

            /* 46: if i != 25 goto nextloop */
            DUP, PUSH, num<100>, SUB, JNZ, addr<main_addr + 4>, POP,
            /* exit i */
            HLT
        >
    >::value opcodes;
    typedef list_t<num<0>> stack;
    printf("Returned: %d\n", exec<opcodes, addr<0>, stack>());
    return 0;
}