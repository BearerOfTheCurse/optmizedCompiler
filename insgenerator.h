#ifndef INSGENERATOR_H
#define INSGENERATOR_H
#include <string>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"

class InsGenerator{
private:

  SymbolTable* myTable;
  InstructionSequence* insSet;

  int insVRIdx;  // the vrIdx in one instruction
  int labelIdx;   //the index for labels


  void visitOperator(struct Node *ast);
  void visit_compare(struct Node *ast, Operand* label, bool isReverse);

public:
  InsGenerator();
  InsGenerator(SymbolTable* table,InstructionSequence* insSet);
  virtual ~InsGenerator();

  void visit(struct Node *ast);

  virtual void visit_program(struct Node *ast);
  virtual void visit_declarations(struct Node *ast);
  virtual void visit_named_type(struct Node *ast);
  virtual void visit_array_type(struct Node *ast);
  virtual void visit_record_type(struct Node *ast);
  virtual void visit_add(struct Node *ast);
  virtual void visit_subtract(struct Node *ast);
  virtual void visit_multiply(struct Node *ast);
  virtual void visit_divide(struct Node *ast);
  virtual void visit_modulus(struct Node *ast);
  virtual void visit_negate(struct Node *ast);
  virtual void visit_int_literal(struct Node *ast);
  virtual void visit_instructions(struct Node *ast);
  virtual void visit_assign(struct Node *ast);
  virtual void visit_if(struct Node *ast);
  virtual void visit_if_else(struct Node *ast);
  virtual void visit_repeat(struct Node *ast);
  virtual void visit_while(struct Node *ast);
  virtual void visit_write(struct Node *ast);
  virtual void visit_read(struct Node *ast);
  Type* visit_var_ref(struct Node *ast);
  Type* visit_array_element_ref(struct Node *ast);
  Type* visit_field_ref(struct Node *ast);
  Type* visit_identifier(struct Node *ast);

  virtual void recur_on_children(struct Node *ast);
};

#endif // ASTVISITOR_H



