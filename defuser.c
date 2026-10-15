#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t low = (uint32_t)challenge;
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t step2 = (low + UINT32_C(0x47c7)) ^ high;
    uint32_t step0 = (middle ^ UINT32_C(0x2720)) + 2u * low;
    values[0] = 5u * (step0 - step2) + UINT32_C(0xa79c);
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    values[0] = (uint32_t)((challenge >> 2) & UINT64_C(0x3f)) + 1u;
    for (size_t i = 1; i < count; ++i) {
        uint32_t nibble = (uint32_t)((challenge >> (23 + i)) & UINT64_C(0xf));
        values[i] = (2u * values[i - 1] ^ nibble) + 13u * (uint32_t)i;
    }
    return write_answer(out, cap, values, count);
}

static uint32_t phase_3_case(unsigned selector, uint32_t value) {
    switch (selector) {
        case 0: return (4u * value) ^ UINT32_C(0x1e1d);
        case 1: return (value ^ UINT32_C(0xe542)) + (value >> 2);
        case 2: return 5u * value + UINT32_C(0x3c09b);
        case 3: return 5u * value + UINT32_C(0x1fa3b);
        case 4: return 5u * value + UINT32_C(0x14e0);
        case 5: return 5u * value + UINT32_C(0x7999);
        case 6: return (2u * value) ^ UINT32_C(0x520f);
        default: return (value ^ UINT32_C(0xd43d)) + (value >> 2);
    }
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    unsigned selector = (unsigned)(challenge & UINT64_C(7));
    uint32_t case_input = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = selector;
    values[1] = phase_3_case(selector, case_input);
    return write_answer(out, cap, values, count);
}

static uint32_t phase_4_walk(uint32_t target, uint32_t low, uint32_t high,
                             uint32_t salt) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + UINT32_C(0x47)) ^ salt;

    uint32_t branch = middle + salt;
    if (target < middle) {
        uint32_t result = phase_4_walk(target, low, middle - 1u, salt);
        return (5u * result ^ branch) ^ UINT32_C(0x7);
    }
    uint32_t result = phase_4_walk(target, middle + 1u, high, salt);
    return (5u * result ^ branch) ^ UINT32_C(0x1e);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    uint32_t target = (uint32_t)challenge & UINT32_C(0x7f);
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase_4_walk(target, 0u, 127u, salt);
    return write_answer(out, cap, values, count);
}

struct phase_5_node {
    int next;
    uint32_t value;
    uint32_t salt;
};

/* p5_nodes in ascending address order; each node is 16 bytes in the bomb. */
static const struct phase_5_node phase_5_nodes[] = {
    {11, 0x412, 0x5140}, {4, 0x782, 0x5578},
    {5, 0x626, 0x173e}, {0, 0x90a, 0x4c3a},
    {7, 0x27a, 0x17c9}, {10, 0x134, 0x66e9},
    {1, 0xa45, 0x2764}, {3, 0x4f, 0xc4ba},
    {-1, 0x83a, 0xf613}, {8, 0xb5b, 0xc9d8},
    {9, 0x52c, 0x50bc}, {2, 0x37e, 0x94ad},
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    static const int heads[8] = {6, 1, 4, 7, 3, 0, 11, 2};
    int node = heads[challenge & UINT64_C(7)];
    unsigned steps = (unsigned)((challenge >> 3) & UINT64_C(3));
    uint32_t accumulator = (uint32_t)(challenge >> 6)
                           ^ (uint32_t)(challenge >> 32) ^ UINT32_C(0x6645);
    for (unsigned i = 0; i <= steps; ++i) {
        accumulator = 3u * accumulator + phase_5_nodes[node].salt;
        accumulator ^= phase_5_nodes[node].value;
        values[1] = phase_5_nodes[node].value;
        if (i < steps) node = phase_5_nodes[node].next;
    }
    values[0] = accumulator;
    return write_answer(out, cap, values, count);
}

static uint32_t phase_6_apply(unsigned operation, uint32_t value) {
    switch (operation) {
        case 0: return (value ^ UINT32_C(0xd738)) + 8u * value;
        case 1: return ((value << 5) | (value >> 27)) ^ UINT32_C(0xc8ab);
        case 2: return ((value << 5) + UINT32_C(0xa982)) ^ (7u * value);
        case 3: return (value + UINT32_C(0x9835)) ^ (value >> 3);
        case 4: return (value >> 3) | (value << 29);
        default: return ((value >> 2) ^ value) + UINT32_C(0x1ed1);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入 values，并把 count 改为整数个数。
    static const unsigned operations[8] = {1, 4, 1, 0, 5, 2, 5, 3};
    uint32_t input = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    unsigned first = (unsigned)(challenge & UINT64_C(7));
    unsigned second = (unsigned)((challenge >> 3) & UINT64_C(7));
    uint32_t intermediate = phase_6_apply(operations[first], input) ^ UINT32_C(0xc04f);
    values[0] = phase_6_apply(operations[second], intermediate) + UINT32_C(0x23de);
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 前 32 字节填充局部缓冲区，OPEN 覆盖相邻标记，后面接三个十进制整数。
    static const char prefix[] = "AAAAAAAA" "AAAAAAAA" "AAAAAAAA" "AAAAAAAA" "OPEN";
    uint32_t first = (uint32_t)context & UINT32_C(3);
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t mixed = challenge_fold ^ context_fold;
    uint32_t rotated = (mixed << 1) | (mixed >> 31);
    uint32_t second = ((rotated + UINT32_C(0xa884)) ^ UINT32_C(0x2a2))
                      ^ (first * UINT32_C(0xa884) ^ (uint32_t)(context >> 16));
    uint32_t third = second ^ (UINT32_C(0x2a2) + context_fold + first
                               + (challenge_fold >> 1));

    int length = snprintf(out, cap, "%s%" PRIu32 " %" PRIu32 " %" PRIu32,
                          prefix, first, second, third);
    return length < 0 || (size_t)length >= cap;
}
