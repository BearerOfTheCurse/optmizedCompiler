#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H
#include <string>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "x86_64.h"

class CodeGenerator
{

private:
  SymbolTable *myTable;
  InstructionSequence *inSeq;
  // convenient shorthands for rsp, rdi, rsi, and r10
  Operand rsp;
  Operand rdi;
  Operand rsi;;
  Operand r10; 
  Operand r11; 
  Operand rax; 
  Operand rdx; 


  // convenient shortcuts for referenced labels
  Operand readCommand;
  Operand scanf_label;

  void nop(Instruction *ins);
  void load_iconst(Instruction *ins);

  void add(Instruction *ins);
  void sub(Instruction *ins);
  void mul(Instruction *ins);
  void div(Instruction *ins);
  void mod(Instruction *ins);
  void negate(Instruction *ins);

  void localAddr(Instruction *ins);
  void loadInt(Instruction *ins);
  void storeInt(Instruction *ins);
  void readi(Instruction *ins);
  void writei(Instruction *ins);

  void jmp(Instruction *ins);
  void je(Instruction *ins);
  void jne(Instruction *ins);
  void jlt(Instruction *ins);
  void jlte(Instruction *ins);
  void jgt(Instruction *ins);
  void jgte(Instruction *ins);

  void compare(Instruction *ins);
  void mov(Instruction *ins);

  void oneStatement(Instruction *ins);

public:
  CodeGenerator(SymbolTable *table, InstructionSequence *inSeq);
  void goThrough();
  void printCode();
  InstructionSequence *outSeq;
};

#endif // INSGENERATOR_H