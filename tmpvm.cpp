#include <stdio.h>
#include <typeinfo>
#include <type_traits>

struct number_base {};

template<size_t N>
struct num : number_base {
    static const size_t value = N;
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

template<typename ops, size_t pc, typename stack>
int exec() {
    static_assert(std::is_base_of_v<opcode_base, typename at<pc, ops>::value>, "Unknown opcode");

    if constexpr (std::is_same_v<typename at<pc, ops>::value, NOP>) {
        return exec<ops, pc + 1, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, DUMP>) {
        print_stack<stack>();
        return exec<ops, pc + 1, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, PUSH>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        return exec<ops, pc + 2, typename stack::template cons<typename at<pc + 1, ops>::value>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, POP>) {
        return exec<ops, pc + 1, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, DUP>) {
        return exec<ops, pc + 1, typename stack::template cons<typename stack::head>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, ADD>) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<num<stack::tail::head::value + stack::head::value>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, SUB>) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<num<stack::tail::head::value - stack::head::value>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, MUL>) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<num<stack::tail::head::value * stack::head::value>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, IDIV>) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<num<stack::tail::head::value / stack::head::value>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, MOD>) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<num<stack::tail::head::value % stack::head::value>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, JMP>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        return exec<ops, at<pc + 1, ops>::value::value, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, JNZ>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        return exec<ops, stack::head::value != 0 ? at<pc + 1, ops>::value::value : pc + 2, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, CALL>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        return exec<ops, at<pc + 1, ops>::value::value, typename stack::template cons<num<pc + 2>>>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, RET>) {
        return exec<ops, stack::head::value, typename stack::tail>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, PRINT>) {
        static_assert(std::is_base_of_v<number_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        printf("%zu\n", at<at<pc + 1, ops>::value::value, stack>::value::value);
        return exec<ops, pc + 2, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, PUTCHAR>) {
        static_assert(std::is_base_of_v<char_base, typename at<pc + 1, ops>::value>, "Bad opcode argument");
        printf("%c", at<pc + 1, ops>::value::value);
        return exec<ops, pc + 2, stack>();
    }
    else if constexpr (std::is_same_v<typename at<pc, ops>::value, HLT>) {
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
        list_t<JMP, num<main_addr>>,
        fizz,
        buzz,
        fizzbuzz,
        list_t<
            /* 0: main_addr */
            PUSH, num<0>,
            JMP, num<main_addr + 5>,

            /* 4: nextloop */
            POP,

            /* 5: loop */
            PUSH, num<1>, ADD,

            /* 8: if i % 15 == 0 fizzbuzz() { */
            DUP, PUSH, num<15>, MOD, JNZ, num<main_addr + 19>, CALL, num<fizzbuzz_addr>, POP, JMP, num<main_addr + 46>,
            /* 19: } else { */
            POP,
            /* 20: if i % 5 == 0 buzz() { */
            DUP, PUSH, num<5>, MOD, JNZ, num<main_addr + 31>, CALL, num<buzz_addr>, POP, JMP, num<main_addr + 46>,
            /* 31: } else { */
            POP,
            /* 32: if i % 3 == 0 fizz() { */
            DUP, PUSH, num<3>, MOD, JNZ, num<main_addr + 43>, CALL, num<fizz_addr>, POP, JMP, num<main_addr + 46>,
            /* 43: } else { */
            POP,
            /* 44: print i */
            PRINT, num<0>,
            /* 46: } */

            /* 46: if i != 25 goto nextloop */
            DUP, PUSH, num<100>, SUB, JNZ, num<main_addr + 4>, POP,
            /* exit i */
            HLT
        >
    >::value opcodes;
    typedef list_t<num<0>> stack;
    printf("Returned: %d\n", exec<opcodes, 0, stack>());
    return 0;
}