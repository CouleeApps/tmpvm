#include <stdio.h>
#include <vector>
#include <tuple>

using len_t = size_t;

template <auto ... rest>
struct intstack_t {
    static const len_t count = sizeof...(rest);
    template<auto ... o>
    using cons = intstack_t<o..., rest...>;
    template<auto ... o>
    using snoc = intstack_t<rest..., o...>;
};

template<>
struct intstack_t<> {
    static const len_t count = 0;
    template<auto ... o>
    using cons = intstack_t<o...>;
    template<auto ... o>
    using snoc = intstack_t<o...>;
};

template <auto i, auto ... rest>
struct intstack_t<i, rest...> {
    static const len_t count = sizeof...(rest) + 1;
    static const auto head = i;
    typedef intstack_t<rest...> tail;
    template<auto ... o>
    using cons = intstack_t<o..., i, rest...>;
    template<auto ... o>
    using snoc = intstack_t<i, rest..., o...>;
};

template <size_t idx, typename stack>
struct at {
    static const auto value = at<idx - 1, typename stack::tail>::value;
};

template<typename stack>
struct at<0, stack> {
    static const auto value = stack::head;
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

enum OpCodes {
    NOP,
    DUMP,
    PUSH,
    POP,
    DUP,
    ADD,
    SUB,
    MUL,
    IDIV,
    MOD,
    JMP,
    JNZ,
    CALL,
    RET,
    PRINT,
    PUTCHAR,
    HLT,
};

template<typename ops, size_t pc, typename stack>
int exec() {
    if constexpr (at<pc, ops>::value == NOP) {
        return exec<ops, pc + 1, stack>();
    }
    else if constexpr (at<pc, ops>::value == DUMP) {
        print_stack<stack>();
        return exec<ops, pc + 1, stack>();
    }
    else if constexpr (at<pc, ops>::value == PUSH) {
        return exec<ops, pc + 2, typename stack::template cons<at<pc + 1, ops>::value>>();
    }
    else if constexpr (at<pc, ops>::value == POP) {
        return exec<ops, pc + 1, typename stack::tail>();
    }
    else if constexpr (at<pc, ops>::value == DUP) {
        return exec<ops, pc + 1, typename stack::template cons<stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == ADD) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<stack::tail::head + stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == SUB) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<stack::tail::head - stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == MUL) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<stack::tail::head * stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == IDIV) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<stack::tail::head / stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == MOD) {
        return exec<ops, pc + 1, typename stack::tail::tail::template cons<stack::tail::head % stack::head>>();
    }
    else if constexpr (at<pc, ops>::value == JMP) {
        return exec<ops, at<pc + 1, ops>::value, stack>();
    }
    else if constexpr (at<pc, ops>::value == JNZ) {
        return exec<ops, stack::head != 0 ? at<pc + 1, ops>::value : pc + 2, stack>();
    }
    else if constexpr (at<pc, ops>::value == CALL) {
        return exec<ops, at<pc + 1, ops>::value, typename stack::template cons<pc + 2>>();
    }
    else if constexpr (at<pc, ops>::value == RET) {
        return exec<ops, stack::head, typename stack::tail>();
    }
    else if constexpr (at<pc, ops>::value == PRINT) {
        printf("%d\n", at<at<pc + 1, ops>::value, stack>::value);
        return exec<ops, pc + 2, stack>();
    }
    else if constexpr (at<pc, ops>::value == PUTCHAR) {
        printf("%c", at<pc + 1, ops>::value);
        return exec<ops, pc + 2, stack>();
    }
    else if constexpr (at<pc, ops>::value == HLT) {
        return stack::head;
    }
    else {
    }
}

int main() {
    typedef intstack_t<
        PUTCHAR, 'F', PUTCHAR, 'i', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET
        > fizz;
    typedef intstack_t<
        PUTCHAR, 'B', PUTCHAR, 'u', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET
        > buzz;
    typedef intstack_t<
        PUTCHAR, 'F', PUTCHAR, 'i', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, 'B', PUTCHAR, 'u', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET
        > fizzbuzz;

    typedef intstack_t<
        /* 0: goto main */
        JMP, 46,
        /* 2: fizz() */
        PUTCHAR, 'F', PUTCHAR, 'i', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET,
        /* 13: buzz() */
        PUTCHAR, 'B', PUTCHAR, 'u', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET,
        /* 24: fizzbuzz() */
        PUTCHAR, 'F', PUTCHAR, 'i', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, 'B', PUTCHAR, 'u', PUTCHAR, 'z', PUTCHAR, 'z', PUTCHAR, '\n', RET,

        /* 43: nextloop */
        POP,
        JMP, 48,

        /* 46: main */
        PUSH, 0,

        /* 48: loop: i++ */
        PUSH, 1, ADD,
        /* 51: if i % 15 == 0 fizzbuzz() */
        DUP, PUSH, 15, MOD, JNZ, 62, CALL, 24, POP, JMP, 89, POP,
        /* 62: if i % 5 == 0 buzz() */
        DUP, PUSH, 5, MOD, JNZ, 74, CALL, 13, POP, JMP, 89, POP,
        /* 75: if i % 3 == 0 fizz() */
        DUP, PUSH, 3, MOD, JNZ, 86, CALL, 2, POP, JMP, 89, POP,
        /* 87: print i */
        PRINT, 0,
        /* if i != 25 goto nextloop */
        DUP, PUSH, 25, SUB, JNZ, 43, POP,
        /* exit i */
        HLT
        > opcodes;
    typedef intstack_t<0> stack;
    printf("Returned: %d\n", exec<opcodes, 0, stack>());
}