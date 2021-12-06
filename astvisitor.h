#ifndef ASTVISITOR_H
#define ASTVISITOR_H
#include <string>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "astvisitor.h"
#include "cfg.h"

class ASTVisitor {

private: 
  Type* intType;
  Type* charType;
  bool build;
  SymbolTable* myTable;

  bool isChar; // used to decide whether a result of expression is char or int
  bool isPrint; 
  bool inConstant;

  int symbolIdx;

  void printRecord(Type* input,bool isType);
  void printLast(Type* input, bool isType);
  std::string getLast(Type* input, bool isType);
  std::string getRecord(Type* input, bool isType);
  void printSymbol(Symbol* input);
  Type* visitOperator(struct Node *ast,int &res);
  Type* visit_expression(struct Node *ast,int &res) ;
  void visit_condition(struct Node *ast);
  Operand* createOperand(int tag, int& vrIdx, Type* datatype);


public:
  ASTVisitor();
  ASTVisitor(bool build,SymbolTable* table);
  virtual ~ASTVisitor();

  void setIsPrint(bool input);
  void visit(struct Node *ast);

  virtual void visit_program(struct Node *ast);
  virtual void visit_declarations(struct Node *ast);
  virtual void visit_constant_declarations(struct Node *ast);
  virtual void visit_constant_def(struct Node *ast);
  virtual void visit_type_declarations(struct Node *ast);
  virtual void visit_type_def(struct Node *ast);
  Type* visit_named_type(struct Node *ast);
  Type* visit_array_type(struct Node *ast);
  Type* visit_record_type(struct Node *ast);
  virtual void visit_var_declarations(struct Node *ast);
  virtual void visit_var_def(struct Node *ast);
  Type* visit_add(struct Node *ast,int &res);
  Type* visit_subtract(struct Node *ast,int &res);
  Type* visit_multiply(struct Node *ast,int &res);
  Type* visit_divide(struct Node *ast,int &res);
  Type* visit_modulus(struct Node *ast,int &res);
  Type* visit_negate(struct Node *ast,int &res);
  Type* visit_int_literal(struct Node *ast,int &res);
  virtual void visit_instructions(struct Node *ast);
  virtual void visit_assign(struct Node *ast);
  virtual void visit_if(struct Node *ast);
  virtual void visit_if_else(struct Node *ast);
  virtual void visit_repeat(struct Node *ast);
  virtual void visit_while(struct Node *ast);
  virtual void visit_compare_eq(struct Node *ast);
  virtual void visit_compare_neq(struct Node *ast);
  virtual void visit_compare_lt(struct Node *ast);
  virtual void visit_compare_lte(struct Node *ast);
  virtual void visit_compare_gt(struct Node *ast);
  virtual void visit_compare_gte(struct Node *ast);
  virtual void visit_write(struct Node *ast);
  virtual void visit_read(struct Node *ast);
  virtual void visit_var_ref(struct Node *ast);
  Type* visit_array_element_ref(struct Node *ast);
  Type* visit_field_ref(struct Node *ast);

  virtual void visit_identifier_list(struct Node *ast);
  virtual void visit_expression_list(struct Node *ast);
  Type* visit_identifier(struct Node *ast,int &res);

  virtual void recur_on_children(struct Node *ast);
};

#endif // ASTVISITOR_H
