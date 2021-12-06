#include "irprinter.h"
#include "cfg.h"
#include "highlevel.h"

////////////////////////////////////////////////////////////////////////
// IRPrinter implementation 
////////////////////////////////////////////////////////////////////////
IRPrinter::IRPrinter(InstructionSequence *iseq):PrintInstructionSequence(iseq){
  this->irs = new PrintHighLevelInstructionSequence(iseq);
}

std::string IRPrinter::get_opcode_name(int opcode) {
  return irs->get_opcode_name(opcode);
}

std::string IRPrinter::get_mreg_name(int regnum) {
  return irs->get_mreg_name(regnum);
}