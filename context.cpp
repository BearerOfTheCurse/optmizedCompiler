#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <unordered_map>
#include <iostream>
#include "util.h"
#include "cpputil.h"
#include "node.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "ast.h"
#include "astvisitor.h"
#include "context.h"
#include "cfg.h"
#include "insgenerator.h"
#include "irprinter.h"
#include "codegenerator.h"
#include "mcg.h"
#include "optimizer.h"
#include "highlevel.h"

using namespace std;

////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////

struct Context
{
private:
  SymbolTable *myTable;
  ASTVisitor *visitor;
  InstructionSequence *insSet;
  CodeGenerator *codeGen;

  Mcg* goodCodeGen;
  
  ControlFlowGraph *cfg;
  ControlFlowGraph *xcfg;

  // optimizers
  ValueNumbering* valueNum;
  ConstProp* cprop;
  VRProp* vprop; 
  Cleaner* cleaner;

  bool build;
  struct Node *root;
  Type *intType;
  Type *charType;

  char flag;

  void printSymbol(Symbol *input);
  void printSymbolTable(SymbolTable *curTable);

  void printRecord(Type *input, bool isType);
  void printLast(Type *input, bool isType);

public:
  Context(struct Node *ast);
  ~Context();

  void set_flag(char flag);

  void build_symtab();

  void build_insSet();
  void print_insSet();

  void build_code();
  void print_code();

  void build_good_code();
  void print_good_code();

  void build_high_cfg();
  void print_high_cfg();

  void build_x86_cfg();
  void print_x86_cfg();

  // TODO: additional methods
};

// TODO: helper classes (e.g., SymbolTableBuilder)

////////////////////////////////////////////////////////////////////////
// Context class implementation
////////////////////////////////////////////////////////////////////////

Context::Context(struct Node *ast)
{
  intType = new Type(INTEGER);
  charType = new Type(CHAR);

  myTable = new SymbolTable(intType, charType);
  root = ast;

  insSet = new InstructionSequence();
}

Context::~Context()
{
}

void Context::set_flag(char flag)
{
  this->flag = flag;
}

void Context::build_insSet()
{
  InsGenerator *insgen = new InsGenerator(myTable, insSet);

  insgen->visit(root);
}

void Context::print_insSet()
{
  IRPrinter *printer = new IRPrinter(insSet);
  printer->print();
}

void Context::build_code()
{
  codeGen = new CodeGenerator(myTable, insSet);
  codeGen->goThrough();
}

void Context::print_code()
{
  codeGen->printCode();
}

void Context::build_good_code()
{
  HighLevelControlFlowGraphBuilder cfg_builder(insSet);
  //ControlFlowGraph *cfg = cfg_builder.build(); 
  cfg = cfg_builder.build(); 

  cprop = new ConstProp(cfg);
  cprop->optimize();
  valueNum = new ValueNumbering(cprop->dst);
  valueNum->optimize();
  vprop = new VRProp(valueNum->dst);
  vprop->optimize();

  cleaner = new Cleaner(vprop->dst); 
  // cleaner = new Cleaner(valueNum->dst);  //cleaner = new Cleaner(cprop->dst);
  cleaner->optimize();

  //  cfg = cprop->dst;
   cfg = cleaner->dst;
  // cfg = vprop->dst;
  // cfg = valueNum->dst;

  goodCodeGen = new Mcg(myTable, cfg);
  goodCodeGen->visitCfg();
}

void Context::print_good_code()
{
  goodCodeGen->printCode();
}

void Context::build_symtab()
{
  build = 1;
  visitor = new ASTVisitor(build, myTable);
  if (flag == 's')
  {
    visitor->setIsPrint(1);
  }

  visitor->visit(root);
}

void Context::build_high_cfg()
{

 // HighLevelControlFlowGraphBuilder cfg_builder(insSet);
 //cfg = cfg_builder.build();

  // optimizer
  // cprop = new ConstProp(cfg);
  // cprop->optimize();
  // valueNum = new ValueNumbering(cprop->dst);
  // valueNum->optimize();

}

void Context::print_high_cfg()
{
  // ControlFlowGraph* tmp = cprop->dst;
  //   ControlFlowGraph* tmp = valueNum->dst;
  // HighLevelControlFlowGraphPrinter tmp_printer(tmp);
  // tmp_printer.print();

  InstructionSequence *result_iseq = cfg->create_instruction_sequence();
  PrintHighLevelInstructionSequence* printer = new PrintHighLevelInstructionSequence(result_iseq);
  printer->print();


  cout<<"////////////////////////////////////////////////////////////////////"<<endl;
  HighLevelControlFlowGraphPrinter cfg_printer(cfg);
  cfg_printer.print();
  }

void Context::build_x86_cfg()
{
  //X86_64ControlFlowGraphBuilder xcfg_builder(codeGen->outSeq);
  //xcfg = xcfg_builder.build();
}

void Context::print_x86_cfg()
{
  X86_64ControlFlowGraphPrinter xcfg_printer(goodCodeGen->lowCfg);
  xcfg_printer.print();
}

////////////////////////////////////////////////////////////////////////
// Context API functions
////////////////////////////////////////////////////////////////////////

struct Context *context_create(struct Node *ast)
{
  return new Context(ast);
}

void context_destroy(struct Context *ctx)
{
  delete ctx;
}

void context_set_flag(struct Context *ctx, char flag)
{
  ctx->set_flag(flag);
}

void context_build_symtab(struct Context *ctx)
{
  ctx->build_symtab();
}

void context_build_IR(struct Context *ctx)
{
  ctx->build_insSet();
}

void context_print_IR(struct Context *ctx)
{
  ctx->print_insSet();
}

void context_build_code(struct Context *ctx)
{
  ctx->build_code();
}

void context_print_code(struct Context *ctx)
{
  ctx->print_code();
}

void context_build_high_cfg(struct Context *ctx)
{
  ctx->build_high_cfg();
}

void context_print_high_cfg(struct Context *ctx)
{
  ctx->print_high_cfg();
}

void context_build_x86_cfg(struct Context *ctx)
{
  ctx->build_x86_cfg();
}

void context_print_x86_cfg(struct Context *ctx)
{
  ctx->print_x86_cfg();
}

void context_check_types(struct Context *ctx)
{
}

void context_build_good_code(struct Context *ctx)
{
  ctx->build_good_code();
}

void context_print_good_code(struct Context *ctx)
{
  ctx->print_good_code();
}
