#define SYSCALL(num, funnum) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(funnum) : "cc", "memory")

#define SYSCALL_1ARG(num, funnum, arg1) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(funnum), "c"(arg1) : "cc", "memory")

#define SYSCALL_2ARG(num, funnum, arg1, arg2) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(funnum), "c"(arg1), "d"(arg2) : "cc", "memory")

#define SYSCALL_3ARG(num, funnum, arg1, arg2, arg3) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(funnum), "c"(arg1), "d"(arg2), "S"(arg3) : "cc", "memory")

#define SYSCALL_4ARG(num, funnum, arg1, arg2, arg3, arg4) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(funnum), "c"(arg1), "d"(arg2), "S"(arg3), "D"(arg4) : "cc", "memory")