#include <string>
#include <iostream>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "codegenerator.h"
#include "highlevel.h"
#include "x86_64.h"

using namespace std;

CodeGenerator::CodeGenerator(SymbolTable *table, InstructionSequence *inSeq) : myTable(table), inSeq(inSeq)
{
    outSeq = new InstructionSequence();

    rsp = Operand(OPERAND_MREG, MREG_RSP);
    rdi = Operand(OPERAND_MREG, MREG_RDI);
    rsi = Operand(OPERAND_MREG, MREG_RSI);
    rax = Operand(OPERAND_MREG, MREG_RAX);

    r10 = Operand(OPERAND_MREG, MREG_R10);
    r11 = Operand(OPERAND_MREG, MREG_R11);
    rdx = Operand(OPERAND_MREG, MREG_RDX);

    readCommand = Operand("s_readint_fmt", true);
    scanf_label = Operand("scanf");
}

void CodeGenerator::oneStatement(Instruction *ins)
{
    int op_code = ins->get_opcode();
    switch (op_code)
    {

    case HINS_NOP:
        nop(ins);
        break;
    case HINS_LOAD_ICONST:
        load_iconst(ins);
        break;
    case HINS_INT_ADD:
        add(ins);
        break;
    case HINS_INT_SUB:
        sub(ins);
        break;
    case HINS_INT_MUL:
        mul(ins);
        break;
    case HINS_INT_DIV:
        div(ins);
        break;
    case HINS_INT_MOD:
        mod(ins);
        break;
    case HINS_INT_NEGATE:
        negate(ins);
        break;
    case HINS_LOCALADDR:
        localAddr(ins);
        break;
    case HINS_LOAD_INT:
        loadInt(ins);
        break;
    case HINS_STORE_INT:
        storeInt(ins);
        break;
    case HINS_READ_INT:
        readi(ins);
        break;
    case HINS_WRITE_INT:
        writei(ins);
        break;
    case HINS_JUMP:
        jmp(ins);
        break;
    case HINS_JE:
        je(ins);
        break;
    case HINS_JNE:
        jne(ins);
        break;
    case HINS_JLT:
        jlt(ins);
        break;
    case HINS_JLTE:
        jlte(ins);
        break;
    case HINS_JGT:
        jgt(ins);
        break;
    case HINS_JGTE:
        jgte(ins);
        break;
    case HINS_INT_COMPARE:
        compare(ins);
        break;
    case HINS_MOV:
        mov(ins);
        break;

    default:
        cerr << "ERROR: Unknown high level code type" << endl;
        exit(-1);
    }
}

void CodeGenerator::goThrough()
{

    // begin going through
    int sz = inSeq->get_length();
    Instruction *ins;
    string label;

    for (int i = 0; i < sz; i++)
    {
        ins = inSeq->get_instruction(i);
        if (inSeq->has_label(i))
        {
            // if the instruction has a lable
            outSeq->define_label(inSeq->get_label(i));
            outSeq->add_instruction(new Instruction(MINS_NOP)); // add nop to avoid conflict with other label
        }

        oneStatement(ins);
    }

    // if these is a label at the end of the code
    if (inSeq->has_label_at_end())
    {
        outSeq->define_label(inSeq->get_label_at_end());
        outSeq->add_instruction(new Instruction(MINS_NOP)); // add nop to avoid conflict with other label
    }
}

void CodeGenerator::nop(Instruction *ins)
{
    outSeq->add_instruction(new Instruction(MINS_NOP));
}

void CodeGenerator::load_iconst(Instruction *ins)
{
    int offset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
    Operand dst(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
    int val = ins->get_operand(1).get_int_value();
    Operand src(OPERAND_INT_LITERAL, val);

    Instruction *first = new Instruction(MINS_MOVQ, src, dst);
    first->set_comment("ldci");
    outSeq->add_instruction(first);
}

void CodeGenerator::add(Instruction *ins)
{
    int lOffset, rOffset, dOffset;
    Operand leftOp, rightOp;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        lOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
        leftOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        rOffset = ins->get_operand(2).get_base_reg() * 8 + myTable->structSize;
        rightOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    }

    dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);
    Instruction *first = new Instruction(MINS_MOVQ, des, rax);
    first->set_comment("add");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, leftOp, rax));
    outSeq->add_instruction(new Instruction(MINS_ADDQ, rightOp, rax));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, rax, des));
}
void CodeGenerator::sub(Instruction *ins)
{

    int lOffset, rOffset, dOffset;
    Operand leftOp, rightOp;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        lOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
        leftOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        rOffset = ins->get_operand(2).get_base_reg() * 8 + myTable->structSize;
        rightOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    }

    dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);
    Instruction *first = new Instruction(MINS_MOVQ, des, rax);
    first->set_comment("sub");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, leftOp, rax));
    outSeq->add_instruction(new Instruction(MINS_SUBQ, rightOp, rax));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, rax, des));
}
void CodeGenerator::mul(Instruction *ins)
{
    int lOffset, rOffset, dOffset;
    Operand leftOp, rightOp;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        lOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
        leftOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        rOffset = ins->get_operand(2).get_base_reg() * 8 + myTable->structSize;
        rightOp = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    }

    dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);

    Instruction *first = new Instruction(MINS_MOVQ, des, rax);
    first->set_comment("mul");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, leftOp, rax));
    outSeq->add_instruction(new Instruction(MINS_IMULQ, rightOp, rax));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, rax, des));
}
void CodeGenerator::div(Instruction *ins)
{
    int lOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
    int rOffset = ins->get_operand(2).get_base_reg() * 8 + myTable->structSize;
    int dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand leftOp(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    Operand rightOp(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);

    Instruction *first = new Instruction(MINS_MOVQ, leftOp, rax);
    first->set_comment("div");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_CQTO));
    outSeq->add_instruction(new Instruction(MINS_IDIVQ, rightOp));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, rax, des));
}
void CodeGenerator::mod(Instruction *ins)
{
    int lOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
    int rOffset = ins->get_operand(2).get_base_reg() * 8 + myTable->structSize;
    int dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand leftOp(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    Operand rightOp(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);

    Instruction *first = new Instruction(MINS_MOVQ, leftOp, rax);
    first->set_comment("div");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_CQTO));
    outSeq->add_instruction(new Instruction(MINS_IDIVQ, rightOp));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, rdx, des));
}
void CodeGenerator::negate(Instruction *ins)
{
}

void CodeGenerator::localAddr(Instruction *ins)
{
    int offset0 = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
    int offset1 = ins->get_operand(1).get_int_value();
    Operand localvar0(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset0);
    Operand localvar1(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset1);

    Instruction *first = new Instruction(MINS_LEAQ, localvar1, r10);
    first->set_comment("localaddr");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, r10, localvar0));
}
void CodeGenerator::loadInt(Instruction *ins)
{

    int lOffset, rOffset;
    lOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
    rOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;

    Operand localvar0(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    Operand localvar1(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);

    Operand localvar2(OPERAND_MREG_MEMREF, MREG_R10);

    Instruction *first = new Instruction(MINS_MOVQ, localvar1, r10);
    first->set_comment("ldi");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, localvar2, r10));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, r10, localvar0));
}
void CodeGenerator::storeInt(Instruction *ins)
{
    int lOffset, rOffset;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(0).get_int_value();
    }
    else
    {
        lOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
    }

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(1).get_int_value();
    }
    else
    {
        rOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
    }

    Operand localvar0(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    Operand localvar1(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);

    Operand localvar2(OPERAND_MREG_MEMREF, MREG_R11);

    Instruction *first = new Instruction(MINS_MOVQ, localvar0, r11);
    first->set_comment("sti");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, localvar1, r10));
    outSeq->add_instruction(new Instruction(MINS_MOVQ, r10, localvar2));
}

void CodeGenerator::readi(Instruction *ins)
{
    Operand localvar;
    int offset;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        offset = ins->get_operand(0).get_int_value();
        localvar = Operand(OPERAND_INT_LITERAL, offset);
    }
    else
    {
        offset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
        localvar = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
    }

    Instruction *first = new Instruction(MINS_MOVQ, readCommand, rdi);
    first->set_comment("readi");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_LEAQ, localvar, rsi));
    outSeq->add_instruction(new Instruction(MINS_CALL, scanf_label));
}

void CodeGenerator::writei(Instruction *ins)
{
    Operand writeCommand("s_writeint_fmt", true);
    Operand printf_label("printf");

    Operand localvar;
    int offset;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        offset = ins->get_operand(0).get_int_value();
        localvar = Operand(OPERAND_INT_LITERAL, offset);
    }
    else
    {
        offset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
        localvar = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
    }

    Instruction *first = new Instruction(MINS_MOVQ, localvar, rsi);
    first->set_comment("writei");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, writeCommand, rdi));
    outSeq->add_instruction(new Instruction(MINS_CALL, printf_label));
}

void CodeGenerator::jmp(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JMP, op));
}

void CodeGenerator::je(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JE, op));
}
void CodeGenerator::jne(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JNE, op));
}
void CodeGenerator::jlt(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JL, op));
}

void CodeGenerator::jlte(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JLE, op));
}

void CodeGenerator::jgt(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JG, op));
}

void CodeGenerator::jgte(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    outSeq->add_instruction(new Instruction(MINS_JGE, op));
}

void CodeGenerator::compare(Instruction *ins)
{

    int lOffset, rOffset;
    Operand left, right;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(0).get_int_value();
        left = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        lOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
        left = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, lOffset);
    }

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(1).get_int_value();
        right = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        rOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
        right = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, rOffset);
    }

    Instruction *first = new Instruction(MINS_MOVQ, left, r10);
    first->set_comment("compare");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_CMPQ, right, r10));
}
void CodeGenerator::mov(Instruction *ins)
{
    int sOffset; // = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
    Operand src;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        sOffset = ins->get_operand(1).get_int_value();
        src = Operand(OPERAND_INT_LITERAL, sOffset);
    }
    else
    {
        sOffset = ins->get_operand(1).get_base_reg() * 8 + myTable->structSize;
        src = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, sOffset);
    }

    int dOffset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;

    Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);

    Instruction *first = new Instruction(MINS_MOVQ, src, r10);
    first->set_comment("mov");

    outSeq->add_instruction(first);
    outSeq->add_instruction(new Instruction(MINS_MOVQ, r10, des));
}

void CodeGenerator::printCode()
{
    // print prologue
    printf("\t.section .rodata\n"
           "s_readint_fmt: .string \"%%ld\"\n"

           "s_writeint_fmt: .string \"%%ld\\n\"\n"
           "\t.section .bss\n"
           "\t.align 8\n"

           "s_readbuf: .space 8\n"
           "\t.section .text\n"

           "\t.globl main\n"
           "main:\n");

    int space = myTable->vrIdxMax * 8 + myTable->structSize + 16; // vr size + array&record size
    if (space % 16 == 0)
        space += 8;
    printf("\tsubq $%d, %%rsp\n", space);

    // use printer ...
    PrintX86_64InstructionSequence print_code(outSeq);
    print_code.print();

    // print epilogue
    printf("\taddq $%d, %%rsp\n", space);
    printf("\tmovl $0, %%eax\n"
           "\tret\n");
}
