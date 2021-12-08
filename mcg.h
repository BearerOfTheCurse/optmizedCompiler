#ifndef MCG_H
#define MCG_H
#include <string>
#include <unordered_map>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "x86_64.h"

class Mcg 
{

private:
  SymbolTable *myTable;
  //InstructionSequence *inSeq;
  ControlFlowGraph *highCfg;

  unordered_map<int,int> vrToMr;  //find which mr store the value of vr
  unordered_map<int,int> mrToVr;  //find the mr contains which vr's value
  
  unordered_map<int,int> liveTime;    //record the last instruction that use a vr
  unordered_map<int,int> spill;       //record the memory address that stores the vr value;

  unordered_map<int,Operand*> mr;       // give machine register an order, easy to find them
  int maxMr = 9;
  int insIdx;
  BasicBlock* curBlock;


  // convenient shorthands for rsp, rdi, rsi, and r10
  Operand rsp;
  Operand rbp;
  Operand rdi;
  Operand rsi;
  Operand rax; 
  Operand rbx; 
  Operand rcx; 
  Operand rdx; 

  Operand r8; 
  Operand r9; 
  Operand r10; 
  Operand r11; 
  Operand r12; 
  Operand r13; 
  Operand r14; 
  Operand r15; 

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

//new functions for reg alloc
  //Operand* getMr(int vrIdx);
  void updateLiveTime(BasicBlock* curBlock);
  void visitBlock(BasicBlock* curBlock);
  Operand* getMrUse(int vrIdx);   //return a mr that contains value to use;
  Operand* getMrStore(int vrIdx);   //return a mr that can be used to store;

  void storeCallerMr();
  void loadCallerMr();
  BasicBlock* findBlock(ControlFlowGraph* cfg, unsigned int bbIdx);   // find basic block of lowCfg by using block Idx;
  void cleanMr();       // move the temperal vr into memory at the end of a block
  Operand* getOperand(int vrIdx, bool use); // return a proper operand accourding to its vrIdx



public:
  Mcg(SymbolTable *table, ControlFlowGraph *inGraph);
  void visitCfg();
  void printCode();
  InstructionSequence *outSeq;
  ControlFlowGraph *lowCfg;
};

#endif // MCG_H