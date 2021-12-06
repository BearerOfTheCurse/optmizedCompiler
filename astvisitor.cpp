#include <cassert>
#include <string>
#include <iostream>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "astvisitor.h"
#include "cfg.h"
using namespace std;

ASTVisitor::ASTVisitor()
{
}

ASTVisitor::ASTVisitor(bool build, SymbolTable *table) : build(build), myTable(table)
{
  myTable = table;
  intType = myTable->symbolMap["INTEGER"]->get_type();
  charType = myTable->symbolMap["CHAR"]->get_type();
  isChar = 1;
  isPrint = 0;
  inConstant = 0;
  symbolIdx = 0;
}

ASTVisitor::~ASTVisitor()
{
}

void ASTVisitor::visit(struct Node *ast)
{
  int tag = node_get_tag(ast);
  int foo;
  switch (tag)
  {
  case AST_PROGRAM:
    visit_program(ast);
    break;
  case AST_DECLARATIONS:
    visit_declarations(ast);
    break;
  case AST_CONSTANT_DECLARATIONS:
    visit_constant_declarations(ast);
    break;
  case AST_CONSTANT_DEF:
    visit_constant_def(ast);
    break;
  case AST_TYPE_DECLARATIONS:
    visit_type_declarations(ast);
    break;
  case AST_TYPE_DEF:
    visit_type_def(ast);
    break;
  case AST_NAMED_TYPE:
    visit_named_type(ast);
    break;
  case AST_ARRAY_TYPE:
    visit_array_type(ast);
    break;
  case AST_RECORD_TYPE:
    visit_record_type(ast);
    break;
  case AST_VAR_DECLARATIONS:
    visit_var_declarations(ast);
    break;
  case AST_VAR_DEF:
    visit_var_def(ast);
    break;
  case AST_ADD:
    visit_add(ast, foo);
    break;
  case AST_SUBTRACT:
    visit_subtract(ast, foo);
    break;
  case AST_MULTIPLY:
    visit_multiply(ast, foo);
    break;
  case AST_DIVIDE:
    visit_divide(ast, foo);
    break;
  case AST_MODULUS:
    visit_modulus(ast, foo);
    break;
  case AST_NEGATE:
    visit_negate(ast, foo);
    break;
  case AST_INT_LITERAL:
    visit_int_literal(ast, foo);
    break;
  case AST_INSTRUCTIONS:
    visit_instructions(ast);
    break;
  case AST_ASSIGN:
    visit_assign(ast);
    break;
  case AST_IF:
    visit_if(ast);
    break;
  case AST_IF_ELSE:
    visit_if_else(ast);
    break;
  case AST_REPEAT:
    visit_repeat(ast);
    break;
  case AST_WHILE:
    visit_while(ast);
    break;
  case AST_COMPARE_EQ:
    visit_compare_eq(ast);
    break;
  case AST_COMPARE_NEQ:
    visit_compare_neq(ast);
    break;
  case AST_COMPARE_LT:
    visit_compare_lt(ast);
    break;
  case AST_COMPARE_LTE:
    visit_compare_lte(ast);
    break;
  case AST_COMPARE_GT:
    visit_compare_gt(ast);
    break;
  case AST_COMPARE_GTE:
    visit_compare_gte(ast);
    break;
  case AST_WRITE:
    visit_write(ast);
    break;
  case AST_READ:
    visit_read(ast);
    break;
  case AST_VAR_REF:
    visit_var_ref(ast);
    break;
  case AST_ARRAY_ELEMENT_REF:
    visit_array_element_ref(ast);
    break;
  case AST_FIELD_REF:
    visit_field_ref(ast);
    break;
  case AST_IDENTIFIER_LIST:
    visit_identifier_list(ast);
    break;
  case AST_EXPRESSION_LIST:
    visit_expression_list(ast);
    break;
  case NODE_TOK_IDENT:
    visit_identifier(ast, foo);
    break;
  default:
    assert(false); // unknown AST node type
  }
}
/////////////////
// handle print
////////////////
string typeToString(int tag)
{
  switch (tag)
  {
  case INTEGER:
    return "INTEGER";
    break;

  case CHAR:
    return "CHAR";
    break;

  case ARRAY:
    return "ARRAY";

    break;

  case RECORD:
    return "RECORD";
    break;
  }
  return "Error ";
}

string symbolToString(int tag)
{
  switch (tag)
  {
  case VAR:
    return "VAR";
    break;

  case TYPE:
    return "TYPE";
    break;

  case CONST:
    return "CONST";
    break;
  }
  return "Error ";
}

void ASTVisitor::setIsPrint(bool input)
{
  this->isPrint = input;
}

string ASTVisitor::getRecord(Type *input, bool isType)
{
  SymbolTable *table = input->recordTable;
  unordered_map<string, Symbol *>::iterator it = table->symbolMap.begin();
  int sz = table->symbolMap.size();
  int cnt = sz - 3;
  string str = "";

  int idx = 0;

  str += "RECORD (";

  while (idx < sz - 2)
  {
    while (it != table->symbolMap.end())
    {
      if (it->first != "INTEGER" && it->first != "CHAR" && it->second->idx == idx)
      {
        getLast(it->second->get_type(), (it->second->get_kind() == TYPE));

        if (cnt > 0)
        {
          str += " x ";
          cnt--;
        }
      }
      it++;
    }
    it = table->symbolMap.begin();
    idx++;
  }
  str += ")";
  return str;
}
void ASTVisitor::printRecord(Type *input, bool isType)
{
  SymbolTable *table = input->recordTable;
  unordered_map<string, Symbol *>::iterator it = table->symbolMap.begin();
  int sz = table->symbolMap.size();
  int cnt = sz - 3;

  int idx = 0;

  cout << "RECORD (";
  while (idx < sz - 2)
  {
    while (it != table->symbolMap.end())
    {
      if (it->first != "INTEGER" && it->first != "CHAR" && it->second->idx == idx)
      {
        printLast(it->second->get_type(), (it->second->get_kind() == TYPE));

        if (cnt > 0)
        {
          cout << " x ";
          cnt--;
        }
      }
      it++;
    }
    it = table->symbolMap.begin();
    idx++;
  }

  cout << ")";
}

void ASTVisitor::printLast(Type *input, bool isType)
{
  switch (input->kind)
  {
  case INTEGER:
    cout << "INTEGER";
    break;

  case CHAR:
    cout << "CHAR";
    break;

  case ARRAY:
    if (input->arrayType->kind != RECORD)
    {
      cout << "ARRAY " << input->len << " OF " << typeToString(input->arrayType->kind);
    }
    else
    {
      cout << "ARRAY " << input->len << " OF ";
      printRecord(input->arrayType, isType);
    }
    break;

  case RECORD:
    printRecord(input, isType);

    break;
  }
}

string ASTVisitor::getLast(Type *input, bool isType)
{
  string str;
  switch (input->kind)
  {
  case INTEGER:
    str = "INTEGER";
    break;

  case CHAR:
    str = "CHAR";
    break;

  case ARRAY:
    if (input->arrayType->kind != RECORD)
    {
      str = "ARRAY " + to_string(input->len) + " OF " + typeToString(input->arrayType->kind);
    }
    else
    {
      str = "ARRAY " + to_string(input->len) + " OF " + getRecord(input->arrayType, isType);
    }
    break;

  case RECORD:
    str = getRecord(input, isType);
    break;
  }
  return str;
}

void ASTVisitor::printSymbol(Symbol *input)
{

  cout << "table_size:" << myTable->size << " "; // Debug check size of type

  cout << myTable->depth << ",";
  ;
  cout << symbolToString(input->get_kind()) << ",";
  cout << input->get_name() + ",";

  if (input->get_kind() == TYPE)
    printLast(input->get_type(), 1);
  else
    printLast(input->get_type(), 0);

  cout << " type_size:" << input->get_type()->size; // Debug check size of type
  if (input->get_kind() == VAR)
  {
    cout << " operand address:" << input->getOperand()->get_base_reg(); // Debug check operand Problems Here!!
  }

  cout << endl;
}

/////////////////
// handle print finish
////////////////

void ASTVisitor::visit_program(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_declarations(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_constant_declarations(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_constant_def(struct Node *ast)
{
  inConstant = 1;
  struct Node *ident = node_get_kid(ast, 0); // identifier
  struct Node *exp = node_get_kid(ast, 1);   // expression
  const char *lexeme = node_get_str(ident);
  string varName(lexeme);

  Symbol *repeat = myTable->lookup(varName);
  if (repeat != nullptr)
  {
    struct SourceInfo info = node_get_source_info(ident);
    string eMsg = "Error: Name '" + varName + "' is already defined";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }
  int value;
  Type *dataType = visit_expression(exp, value);

  Symbol *newSymbol;
  newSymbol = new Symbol(varName, CONST, dataType);
  newSymbol->set_ival(value);

  newSymbol->idx = symbolIdx;
  symbolIdx++;

  // add operand to symbol table
  Operand *op = new Operand(OPERAND_INT_LITERAL, value);
  newSymbol->setOperand(op);

  myTable->insert(varName, newSymbol);

  // print
  if (isPrint)
    printSymbol(newSymbol);
  inConstant = 0;
}

void ASTVisitor::visit_type_declarations(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_type_def(struct Node *ast)
{
  struct Node *nameNode = node_get_kid(ast, 0);
  const char *lexeme = node_get_str(nameNode);
  string name(lexeme);

  if (myTable->symbolMap.count(name) != 0)
  {
    struct SourceInfo info = node_get_source_info(nameNode);
    string eMsg = "Error: Name '" + name + "' is already defined";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  Type *dataType;

  struct Node *typeNode = node_get_kid(ast, 1);
  int tag = node_get_tag(typeNode);

  if (tag == AST_NAMED_TYPE)
  {
    dataType = visit_named_type(typeNode);
  }
  else if (tag == AST_ARRAY_TYPE)
  {
    dataType = visit_array_type(typeNode);
  }
  else if (tag == AST_RECORD_TYPE)
  {
    dataType = visit_record_type(typeNode);
  }
  else
  {

    struct SourceInfo info = node_get_source_info(typeNode);
    string eMsg = "Error: Unknown Type ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  Symbol *newSymbol = new Symbol(name, TYPE, dataType);

  newSymbol->idx = symbolIdx;
  symbolIdx++;

  myTable->insert(name, newSymbol);

  if (isPrint)
    printSymbol(newSymbol);
}

Type *ASTVisitor::visit_named_type(struct Node *ast)
{
  struct Node *typeNode = node_get_kid(ast, 0);
  const char *lexeme = node_get_str(typeNode);
  string s(lexeme);
  Symbol *curSymbol = myTable->lookup(s);
  if (curSymbol == nullptr)
  {
    struct SourceInfo info = node_get_source_info(typeNode);
    string eMsg = "Error: Unknown Type: '" + s + "'";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (curSymbol->get_kind() != TYPE)
  {

    struct SourceInfo info = node_get_source_info(typeNode);
    string eMsg = "Error: Identifier '" + s + "' does not refer to a type";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }
  return curSymbol->get_type();
}

Type *ASTVisitor::visit_array_type(struct Node *ast)
{

  struct Node *lenNode = node_get_kid(ast, 0);
  const char *lexeme = node_get_str(lenNode);
  int len;
  visit_expression(lenNode, len);

  struct Node *typeNode = node_get_kid(ast, 1);
  int tag = node_get_tag(typeNode);
  Type *dataType;

  if (tag == AST_NAMED_TYPE)
  {
    dataType = visit_named_type(typeNode);
  }
  else if (tag == AST_ARRAY_TYPE)
  {
    dataType = visit_array_type(typeNode);
  }
  else if (tag == AST_RECORD_TYPE)
  {
    dataType = visit_record_type(typeNode);
  }
  else
  {
    struct SourceInfo info = node_get_source_info(typeNode);
    string eMsg = "Error: Unknown Type ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  return new Type(dataType, len);
}

Type *ASTVisitor::visit_record_type(struct Node *ast)
{
  Type *myInt = myTable->lookup("INTEGER")->get_type();
  Type *myChar = myTable->lookup("CHAR")->get_type();
  SymbolTable *newTable = new SymbolTable(myTable, myInt, myChar);
  ASTVisitor *newVisitor = new ASTVisitor(1, newTable);
  struct Node *newRoot = node_build0(AST_VAR_DECLARATIONS);

  int kidNum = node_get_num_kids(ast);
  for (int i = 0; i < kidNum; i++)
  {
    newRoot->add_kid(ast->get_kid(i));
  }

  newVisitor->setIsPrint(this->isPrint);
  newVisitor->visit(newRoot);
  return new Type(newTable);
}

void ASTVisitor::visit_var_declarations(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

Operand *ASTVisitor::createOperand(int tag, int &vrIdx, Type *datatype)
{

  Operand *res;
  if (datatype->kind == INTEGER || datatype->kind == CHAR)
  {
    res = new Operand(OPERAND_VREG, vrIdx);
    vrIdx++;
  }
  else if (datatype->kind == ARRAY || datatype->kind == RECORD)
  {
    res = new Operand(OPERAND_VREG_MEMREF_OFFSET, myTable->structSize, 0);
    // The size of table will increase when the symbol is inserted, so no need to increase the table size here
  }

  return res;
}

void ASTVisitor::visit_var_def(struct Node *ast)
{
  struct Node *typeNode = node_get_kid(ast, 0);
  Type *dataType;
  int tag = node_get_tag(typeNode);

  if (tag == AST_NAMED_TYPE)
  {
    dataType = visit_named_type(typeNode);
  }
  else if (tag == AST_ARRAY_TYPE)
  {
    dataType = visit_array_type(typeNode);
  }
  else if (tag == AST_RECORD_TYPE)
  {
    dataType = visit_record_type(typeNode);
  }
  else
  {

    struct SourceInfo info = node_get_source_info(typeNode);
    string eMsg = "Error: Unknown Type ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  Symbol *newSymbol;
  int kidNum = node_get_num_kids(ast);
  struct Node *child;

  for (int i = 1; i < kidNum; i++)
  { // Jump the first one, as it defines the type
    child = node_get_kid(ast, i);
    const char *lexeme = node_get_str(child);
    string s(lexeme);

    if (myTable->symbolMap.count(s) != 0)
    {
      struct SourceInfo info = node_get_source_info(child);
      string eMsg = "Error: Name '" + s + "' is already defined";
      cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
      exit(-1);
    }

    newSymbol = new Symbol(s, VAR, dataType);
    // add operand to symbol table
    Operand *op = createOperand(tag, myTable->vrIdx, dataType);
    newSymbol->setOperand(op);

    // handle sequence in printing record
    newSymbol->idx = symbolIdx;
    symbolIdx++;

    myTable->insert(s, newSymbol);

    if (isPrint)
      printSymbol(newSymbol);
  }
}

Type *ASTVisitor::visit_expression(struct Node *ast, int &res)
{
  int tag = node_get_tag(ast);
  if (inConstant)
  {
    if (tag == AST_ARRAY_ELEMENT_REF || tag == AST_FIELD_REF)
    {
      struct SourceInfo info = node_get_source_info(ast);
      string eMsg = "Error: Non-constant in constant expression";
      cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
      exit(-1);
    }
    else if (tag == NODE_TOK_IDENT)
    {
      const char *lexeme = node_get_str(ast);
      string name(lexeme);
      Symbol *sym = myTable->lookup(name);
      if (sym->get_kind() != CONST)
      {

        struct SourceInfo info = node_get_source_info(ast);
        string eMsg = "Error: Non-constant in constant expression";
        cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
        exit(-1);
      }
    }
  }

  switch (tag)
  {
  case AST_ADD:
    return visit_add(ast, res);
    break;
  case AST_SUBTRACT:
    return visit_subtract(ast, res);
    break;
  case AST_MULTIPLY:
    return visit_multiply(ast, res);
    break;
  case AST_DIVIDE:
    return visit_divide(ast, res);
    break;
  case AST_MODULUS:
    return visit_modulus(ast, res);
    break;
  case AST_NEGATE:
    return visit_negate(ast, res);
    break;
  case AST_INT_LITERAL:
    return visit_int_literal(ast, res);
    break;
  case AST_ARRAY_ELEMENT_REF:
    return visit_array_element_ref(ast);
    break;
  case AST_FIELD_REF:
    return visit_field_ref(ast);
    break;
  case NODE_TOK_IDENT:
    return visit_identifier(ast, res);
    break;
  default:
    assert(false); // unknown AST node type
  }
  return nullptr;
}

Type *ASTVisitor::visitOperator(struct Node *ast, int &res)
{
  struct Node *left = node_get_kid(ast, 0);
  struct Node *right = node_get_kid(ast, 1);
  int tag = node_get_tag(ast);
  int leftVal, rightVal;

  Type *dataType1 = visit_expression(left, leftVal);
  Type *dataType2 = visit_expression(right, rightVal);

  if (tag == AST_DIVIDE || tag == AST_MODULUS)
  {
    if (rightVal == 0)
    {
      struct SourceInfo info = node_get_source_info(right);
      string eMsg;
      if (tag == AST_DIVIDE)
      {
        eMsg = "Error: Division by 0 in constant expression";
      }
      else
      {
        eMsg = "Error: Mod by 0 in constant expression";
      }
      cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
      exit(-1);
    }
  }

  switch (tag)
  {
  case AST_ADD:
    res = leftVal + rightVal;
    break;
  case AST_SUBTRACT:
    res = leftVal - rightVal;
    break;
  case AST_MULTIPLY:
    res = leftVal * rightVal;
    break;
  case AST_DIVIDE:
    res = leftVal / rightVal;
    break;
  case AST_MODULUS:
    res = leftVal % rightVal;
    break;
  }

  if (dataType1->kind == ARRAY || dataType1->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(ast);
    string eMsg = "Error: Left operand has inappropriate type '" + getLast(dataType2, 0) + "'";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (dataType2->kind == ARRAY || dataType2->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(ast);
    string eMsg = "Error: Right operand has inappropriate type '" + getLast(dataType2, 0) + "'";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (dataType1->kind == dataType2->kind)
  {
    return dataType1;
  }
  else
  {
    if (dataType1->kind == INTEGER)
      return dataType1;
    else
      return dataType2;
  }
}

Type *ASTVisitor::visit_add(struct Node *ast, int &res)
{
  return visitOperator(ast, res);
}

Type *ASTVisitor::visit_subtract(struct Node *ast, int &res)
{
  return visitOperator(ast, res);
}

Type *ASTVisitor::visit_multiply(struct Node *ast, int &res)
{
  return visitOperator(ast, res);
}

Type *ASTVisitor::visit_divide(struct Node *ast, int &res)
{
  return visitOperator(ast, res);
}

Type *ASTVisitor::visit_modulus(struct Node *ast, int &res)
{
  return visitOperator(ast, res);
}

Type *ASTVisitor::visit_negate(struct Node *ast, int &res)
{
  struct Node *num = node_get_kid(ast, 1);
  Type *dataType = visit_expression(num, res);

  struct Node *sign = node_get_kid(ast, 0);

  if (node_get_tag(sign) == NODE_TOK_MINUS)
  {
    res = -res;
  }

  return dataType;
}

Type *ASTVisitor::visit_int_literal(struct Node *ast, int &res)
{
  this->isChar = 0;
  const char *lexeme = node_get_str(ast);
  res = atol(lexeme);

  Symbol *sym = myTable->lookup("INTEGER");
  return sym->get_type();
}

void ASTVisitor::visit_instructions(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_assign(struct Node *ast)
{
  struct Node *var = node_get_kid(ast, 0);
  struct Node *value = node_get_kid(ast, 1);
  int foo;
  Type *varType = visit_expression(var, foo);
  Type *valType = visit_expression(value, foo);

  if (varType->kind == ARRAY || varType->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(var);
    string eMsg = "Error: datatype of variable in assign is not correct!  ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (valType->kind == ARRAY || valType->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(value);
    string eMsg = "Error: datatype of value in assign is not correct!  ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (varType->kind != valType->kind)
  {

    struct SourceInfo info = node_get_source_info(ast);
    string eMsg = "Error: datatypes of value and variable are different in assign ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }
}

void ASTVisitor::visit_if(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_if_else(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_repeat(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_while(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_condition(struct Node *ast)
{
  struct Node *left = node_get_kid(ast, 0);
  struct Node *right = node_get_kid(ast, 1);
  int foo;
  Type *leftType = visit_expression(left, foo);
  Type *rightType = visit_expression(right, foo);

  if (leftType->kind == ARRAY || leftType->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(left);
    string eMsg = "Error: Datatype of left operand in condition is incorrect!  ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (rightType->kind == ARRAY || rightType->kind == RECORD)
  {

    struct SourceInfo info = node_get_source_info(right);
    string eMsg = "Error: Datatype of right operand in condition is incorrect!  ";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }
}

void ASTVisitor::visit_compare_eq(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_compare_neq(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_compare_lt(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_compare_lte(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_compare_gt(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_compare_gte(struct Node *ast)
{
  visit_condition(ast);
}

void ASTVisitor::visit_write(struct Node *ast)
{
  visit_read(ast);
}

void ASTVisitor::visit_read(struct Node *ast)
{
  struct Node *varNode = node_get_kid(ast, 0);
  int foo;
  Type *varType = visit_expression(varNode, foo);

  if (varType->kind == INTEGER || varType->kind == CHAR)
  {
    return;
  }
  if (varType->kind == ARRAY && varType->arrayType->kind == CHAR)
  {
    return;
  }

  struct SourceInfo info = node_get_source_info(varNode);
  string eMsg = "Error: Operand of READ has inappropriate type " + getLast(varType, 0);
  cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
  exit(-1);
}

void ASTVisitor::visit_var_ref(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

Type *ASTVisitor::visit_array_element_ref(struct Node *ast)
{
  struct Node *arrayNode = node_get_kid(ast, 0);
  struct Node *idxNode = node_get_kid(ast, 1);

  const char *lexeme = node_get_str(arrayNode);
  string arrayName(lexeme);

  int idx;

  Type *curType = visit_expression(arrayNode, idx);
  Type *idxType = visit_expression(idxNode, idx);

  if (curType->kind != ARRAY)
  {

    struct SourceInfo info = node_get_source_info(arrayNode);
    string eMsg = "Error: Type '" + typeToString(curType->kind) + "' is not an array type";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  if (idxType->kind == ARRAY || idxType->kind == RECORD)
  {
    struct SourceInfo info = node_get_source_info(idxNode);
    string eMsg = "Error: Subscript expression has non-integral type '" + getLast(idxType, 0) + "'";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  return curType->arrayType;
}

Type *ASTVisitor::visit_field_ref(struct Node *ast)
{
  struct Node *areaNode = node_get_kid(ast, 0);
  struct Node *targetNode = node_get_kid(ast, 1);

  int foo;
  Type *areaType = visit_expression(areaNode, foo);
  const char *lexeme = node_get_str(areaNode);
  if (areaType->kind != RECORD)
  {
    struct SourceInfo info = node_get_source_info(areaNode);
    string eMsg = "Error: Type '" + typeToString(areaType->kind) + "' is not a record type";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  lexeme = node_get_str(targetNode);
  string targetName(lexeme);
  SymbolTable *recordTable = areaType->recordTable;
  Symbol *dataSymbol = recordTable->lookup(targetName);

  if (dataSymbol == nullptr)
  {
    struct SourceInfo info = node_get_source_info(areaNode);
    string eMsg = "Error: Field '" + targetName + "' does not exist in type " + getLast(areaType, 0);
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  return dataSymbol->get_type();
}

void ASTVisitor::visit_identifier_list(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void ASTVisitor::visit_expression_list(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

Type *ASTVisitor::visit_identifier(struct Node *ast, int &res)
{
  const char *lexeme = node_get_str(ast);
  string varName(lexeme);
  if (!myTable->symbolMap.count(varName))
  {
    struct SourceInfo info = node_get_source_info(ast);
    string eMsg = "Error: Undefined variable: '" + varName + "'";
    cerr << info.filename << ":" << info.line << ":" << info.col << ": " << eMsg << endl;
    exit(-1);
  }

  Symbol *curSymbol = myTable->symbolMap[varName];
  if (curSymbol->get_kind() == CONST)
  {
    res = curSymbol->get_ival();
  }

  return curSymbol->get_type();
}

void ASTVisitor::recur_on_children(struct Node *ast)
{
  int num_kids = node_get_num_kids(ast);
  for (int i = 0; i < num_kids; i++)
  {
    visit(node_get_kid(ast, i));
  }
}
