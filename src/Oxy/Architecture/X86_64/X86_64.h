#ifndef OXYIR_X86_64INSTRUCTIONS_H
#define OXYIR_X86_64INSTRUCTIONS_H

#include <cstdint>

namespace Oxy::Architecture::X86_64 {
    enum class Opcode : uint32_t {
        // Data movement
        MOV, LEA,

        // Arithmetic
        ADD, SUB, IMUL, IDIV, NEG, INC, DEC,
        ADDSS, SUBSS, MULSS, DIVSS, // f32
        ADDSD, SUBSD, MULSD, DIVSD, // f64

        // Logical
        AND, OR, XOR, NOT, SHL, SHR, SAR,

        // Comparison & branching
        CMP, TEST, JMP, JE, JNE, JL, JLE, JG, JGE,

        // Stack
        PUSH, POP,

        // Functions
        CALL, RET,

        // Conversion
        CVTSI2SS, CVTSI2SD, CVTSS2SI, CVTSD2SI, CVTSS2SD, CVTSD2SS,

        // Setcc instructions for setting a byte based on a condition
        SETE, SETNE, SETL, SETLE, SETG, SETGE, SETA, SETAE, SETB, SETBE,

        // Other
        CDQE, CQO, NOP,
    };

    enum class Register : uint32_t {
        RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP,
        R8, R9, R10, R11, R12, R13, R14, R15,
        XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7
    };
}

#endif //OXYIR_X86_64INSTRUCTIONS_H
