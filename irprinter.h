#ifndef IRPRINTER_H
#define IRPRINTER_H

#include "cfg.h"
#include "highlevel.h"
using namespace std;


class IRPrinter : public PrintInstructionSequence {
  PrintHighLevelInstructionSequence *irs;
  public:
      IRPrinter(InstructionSequence *iseq);
      string get_opcode_name(int opcode);
      string get_mreg_name(int regnum);
};

#endif //IRPRINTER_H 