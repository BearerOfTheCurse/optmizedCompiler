#include <cassert>
#include "cfg.h"
#include "highlevel.h"

PrintHighLevelInstructionSequence::PrintHighLevelInstructionSequence(InstructionSequence *ins)
  : PrintInstructionSequence(ins) {
}

std::string PrintHighLevelInstructionSequence::get_opcode_name(int opcode) {
  switch (opcode) {
  case HINS_NOP:         return "nop";
  case HINS_LOAD_ICONST: return "ldci";
  case HINS_INT_ADD:     return "addi";
  case HINS_INT_SUB:     return "subi";
  case HINS_INT_MUL:     return "muli";
  case HINS_INT_DIV:     return "divi";
  case HINS_INT_MOD:     return "modi";
  case HINS_INT_NEGATE:  return "negi";
  case HINS_LOCALADDR:   return "localaddr";
  case HINS_LOAD_INT:    return "ldi";
  case HINS_STORE_INT:   return "sti";
  case HINS_READ_INT:    return "readi";
  case HINS_WRITE_INT:   return "writei";
  case HINS_JUMP:        return "jmp";
  case HINS_JE:          return "je";
  case HINS_JNE:         return "jne";
  case HINS_JLT:         return "jlt";
  case HINS_JLTE:        return "jlte";
  case HINS_JGT:         return "jgt";
  case HINS_JGTE:        return "jgte";
  case HINS_INT_COMPARE: return "cmpi";
  case HINS_MOV:         return "mov";
  case HINS_LEA:         return "lea";

  default:
    assert(false);
    return "<invalid>";
  }
}


std::string PrintHighLevelInstructionSequence::get_mreg_name(int regnum) {
  // high level instructions should not use machine registers
  assert(false);
  return "<invalid>";
}
HighLevelControlFlowGraphBuilder::HighLevelControlFlowGraphBuilder(InstructionSequence *iseq)
  : ControlFlowGraphBuilder(iseq) {
}

HighLevelControlFlowGraphBuilder::~HighLevelControlFlowGraphBuilder() {
}

// It's necessary to override this method for x86-64, because call instructions
// have a single label as an Operand, but for our purposes, should not be considered
// as a branch.
// bool HighLevelControlFlowGraphBuilder::is_branch(Instruction *ins) {
//   if (ins->get_opcode() == HINS_READ_INT || ins->get_opcode() == HINS_WRITE_INT) {
//     return false;
//   }
//   return ControlFlowGraphBuilder::is_branch(ins);
// }

bool HighLevelControlFlowGraphBuilder::falls_through(Instruction *ins) {
  // only the jmp instruction does not fall through
  return ins->get_opcode() != HINS_JUMP;
}

HighLevelControlFlowGraphPrinter::HighLevelControlFlowGraphPrinter(ControlFlowGraph *cfg)
  : ControlFlowGraphPrinter(cfg) {
}

HighLevelControlFlowGraphPrinter::~HighLevelControlFlowGraphPrinter() {
}

void HighLevelControlFlowGraphPrinter::print_basic_block(BasicBlock *bb) {
  PrintHighLevelInstructionSequence print_iseq(bb);
  print_iseq.print();
}

