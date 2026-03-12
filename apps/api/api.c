#define SYSCALL(num) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num) : "cc", "memory")

#define SYSCALL_1ARG(num, arg1) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(arg1) : "cc", "memory")

#define SYSCALL_2ARG(num, arg1, arg2) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(arg1), "c"(arg2) : "cc", "memory")

#define SYSCALL_3ARG(num, arg1, arg2, arg3) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3) : "cc", "memory")

#define SYSCALL_4ARG(num, arg1, arg2, arg3, arg4) \
    asm volatile ("int $0x80" : "=a"(__ret) : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4) : "cc", "memory")