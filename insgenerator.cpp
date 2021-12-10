#include <cassert>
#include <string>
#include <iostream>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "insgenerator.h"
#include "cfg.h"
#include "highlevel.h"
using namespace std;

InsGenerator::InsGenerator()
{
}

InsGenerator::InsGenerator(SymbolTable *table, InstructionSequence *insSet) : myTable(table), insSet(insSet)
{
  labelIdx = 0; // the index for labels
  insVRIdx = myTable->vrIdx;
}

InsGenerator::~InsGenerator()
{
}

void InsGenerator::visit(struct Node *ast)
{
  int tag = node_get_tag(ast);
  switch (tag)
  {
  case AST_PROGRAM:
    visit_program(ast);
    break;
  case AST_DECLARATIONS:
    visit_declarations(ast);
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
  case AST_ADD:
    visit_add(ast);
    break;
  case AST_SUBTRACT:
    visit_subtract(ast);
    break;
  case AST_MULTIPLY:
    visit_multiply(ast);
    break;
  case AST_DIVIDE:
    visit_divide(ast);
    break;
  case AST_MODULUS:
    visit_modulus(ast);
    break;
  case AST_NEGATE:
    visit_negate(ast);
    break;
  case AST_INT_LITERAL:
    visit_int_literal(ast);
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
  case NODE_TOK_IDENT:
    visit_identifier(ast);
    break;
  default:
    assert(false); // unknown AST node type
  }
}

void InsGenerator::visit_program(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void InsGenerator::visit_declarations(struct Node *ast)
{
  return;
}

void InsGenerator::visit_named_type(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void InsGenerator::visit_array_type(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void InsGenerator::visit_record_type(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void InsGenerator::visitOperator(struct Node *ast)
{
  recur_on_children(ast);

  int tag = node_get_tag(ast);
  struct Node *leftChild = node_get_kid(ast, 0);
  struct Node *rightChild = node_get_kid(ast, 1);
  Operand *leftOperand = leftChild->getOperand();
  Operand *rightOperand = rightChild->getOperand();

  Operand *res = new Operand(OPERAND_VREG, insVRIdx);
  insVRIdx++;
  Instruction *ins;

  // if left Operand is an address
  int leftTag = node_get_tag(leftChild);
  if (leftTag == AST_ARRAY_ELEMENT_REF || leftTag == AST_FIELD_REF)
  {
    ins = new Instruction(HINS_LOAD_INT, *res, *leftOperand);
    insSet->add_instruction(ins);
    leftOperand = res;
    res = new Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
  }

  // if right Operand is an address
  int rightTag = node_get_tag(rightChild);
  if (rightTag == AST_ARRAY_ELEMENT_REF || rightTag == AST_FIELD_REF)
  {
    ins = new Instruction(HINS_LOAD_INT, *res, *rightOperand);
    rightOperand = res;
    insSet->add_instruction(ins);
    //   insVRIdx++;
    res = new Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
  }

  switch (tag)
  {
  case AST_ADD:
    ins = new Instruction(HINS_INT_ADD, *res, *leftOperand, *rightOperand);
    break;
  case AST_SUBTRACT:
    ins = new Instruction(HINS_INT_SUB, *res, *leftOperand, *rightOperand);
    break;
  case AST_MULTIPLY:
    ins = new Instruction(HINS_INT_MUL, *res, *leftOperand, *rightOperand);
    break;
  case AST_DIVIDE:
    ins = new Instruction(HINS_INT_DIV, *res, *leftOperand, *rightOperand);
    break;
  case AST_MODULUS:
    ins = new Instruction(HINS_INT_MOD, *res, *leftOperand, *rightOperand);
    break;

  default:
    cerr << "Error: Unknown operator" << endl;
    exit(-1);
  }
  ast->setOperand(res);
  insSet->add_instruction(ins);
}

void InsGenerator::visit_add(struct Node *ast)
{
  visitOperator(ast);
}

void InsGenerator::visit_subtract(struct Node *ast)
{
  visitOperator(ast);
}

void InsGenerator::visit_multiply(struct Node *ast)
{
  visitOperator(ast);
}

void InsGenerator::visit_divide(struct Node *ast)
{
  visitOperator(ast);
}

void InsGenerator::visit_modulus(struct Node *ast)
{
  visitOperator(ast);
}

void InsGenerator::visit_negate(struct Node *ast)
{
  struct Node *leftChild = node_get_kid(ast, 0);
  struct Node *rightChild = node_get_kid(ast, 1);
  visit(rightChild);
  Operand *rightOperand = rightChild->getOperand();

  if (node_get_tag(leftChild) == NODE_TOK_PLUS)
  {
    ast->setOperand(rightOperand);
  }
  else
  {
    Operand *op1 = new Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
    Operand op2 = Operand(OPERAND_INT_LITERAL, 0);

    insSet->add_instruction(new Instruction(HINS_INT_SUB, *op1, op2, *rightOperand));
    ast->setOperand(op1);
  }
}

void InsGenerator::visit_int_literal(struct Node *ast)
{
  const char *lexeme = node_get_str(ast);
  string value(lexeme);
  int val = stoi(value);

  Operand op2;                                        // if the op is only used in local, no need to new
  Operand *op1 = new Operand(OPERAND_VREG, insVRIdx); // if the op need to be used outside the function, must use new
  insVRIdx++;
  op2 = Operand(OPERAND_INT_LITERAL, val);
  insSet->add_instruction(new Instruction(HINS_LOAD_ICONST, *op1, op2));
  ast->setOperand(op1);
}

void InsGenerator::visit_instructions(struct Node *ast)
{
  recur_on_children(ast); // default behavior
}

void InsGenerator::visit_assign(struct Node *ast)
{
  //insVRIdx = myTable->vrIdx;
  recur_on_children(ast); // default behavior

  struct Node *leftChild = node_get_kid(ast, 0);
  struct Node *rightChild = node_get_kid(ast, 1);
  Operand *leftOperand = leftChild->getOperand();
  Operand *rightOperand = rightChild->getOperand();

  Instruction *ins;
  // right side of = is an array element/ record field
  int tag;
  tag = node_get_tag(rightChild);
  if (tag == AST_ARRAY_ELEMENT_REF || tag == AST_FIELD_REF)
  {
    Operand op = Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
    insSet->add_instruction(new Instruction(HINS_LOAD_INT, op, *rightOperand));
    rightOperand = &op;
  }

  tag = node_get_tag(leftChild);
  if (tag == AST_ARRAY_ELEMENT_REF || tag == AST_FIELD_REF)
  {
    ins = new Instruction(HINS_STORE_INT, *leftOperand, *rightOperand);
  }
  else
  {
    ins = new Instruction(HINS_MOV, *leftOperand, *rightOperand);
  }

  insSet->add_instruction(ins);

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);

  //insVRIdx = myTable->vrIdx;
}

void InsGenerator::visit_if(struct Node *ast)
{
  struct Node *left = node_get_kid(ast, 0);  // left is condition
  struct Node *right = node_get_kid(ast, 1); // right is body

  string label = ".L" + to_string(labelIdx);
  labelIdx++;
  Operand *op2 = new Operand(label, 0);

  visit_compare(left, op2, 1);

  visit(right);

  insSet->define_label(label);
  insSet->add_instruction(new Instruction(HINS_NOP)); // set a nop to avoid conflict label setting with outside if

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);
}

void InsGenerator::visit_if_else(struct Node *ast)
{
  struct Node *condition = node_get_kid(ast, 0);
  struct Node *left = node_get_kid(ast, 1);
  struct Node *right = node_get_kid(ast, 2);

  string label0 = ".L" + to_string(labelIdx);
  labelIdx++;
  string label1 = ".L" + to_string(labelIdx);
  labelIdx++;

  Operand *op0 = new Operand(label0, 0);
  Operand *op1 = new Operand(label1, 0);

  visit_compare(condition, op0, 1);

  visit(left);
  insSet->add_instruction(new Instruction(HINS_JUMP, *op1));

  insSet->define_label(label0);

  visit(right);

  insSet->define_label(label1);

  insSet->add_instruction(new Instruction(HINS_NOP)); // set a nop to avoid conflict label setting with outside if

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);
}

void InsGenerator::visit_repeat(struct Node *ast)
{
  struct Node *left = node_get_kid(ast, 0);
  struct Node *right = node_get_kid(ast, 1);

  string label = ".L" + to_string(labelIdx);
  labelIdx++;
  insSet->define_label(label);

  Operand *op2 = new Operand(label, 0);

  visit(left);

  visit_compare(right, op2, 1);

  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);
}

void InsGenerator::visit_while(struct Node *ast)
{
  struct Node *left = node_get_kid(ast, 0);
  struct Node *right = node_get_kid(ast, 1);

  // jump to condition
  string label0 = ".L" + to_string(labelIdx);
  labelIdx++;
  Operand op1 = Operand(label0, 0);
  insSet->add_instruction(new Instruction(HINS_JUMP, op1));

  string label1 = ".L" + to_string(labelIdx);
  labelIdx++;

  Operand *op2 = new Operand(label1, 0);

  insSet->define_label(label1);

  visit(right);

  insSet->define_label(label0);

  visit_compare(left, op2, 0);

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);
}

void InsGenerator::visit_compare(struct Node *ast, Operand *label, bool isReverse)
{
  //insVRIdx = myTable->vrIdx;
  recur_on_children(ast); // default behavior

  struct Node *left = node_get_kid(ast, 0);
  struct Node *right = node_get_kid(ast, 1);
  int tag, ltag, rtag;
  tag = node_get_tag(ast);
  ltag = node_get_tag(left);
  rtag = node_get_tag(right);

  Operand op1, op2, op3;
  op1 = *(left->getOperand());
  op2 = *(right->getOperand());

  // if left operand is an address
  if (ltag == AST_ARRAY_ELEMENT_REF || ltag == AST_FIELD_REF)
  {
    op3 = Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
    insSet->add_instruction(new Instruction(HINS_LOAD_INT, op3, op1));
    op1 = op3;
  }

  // if right operand is an address
  if (rtag == AST_ARRAY_ELEMENT_REF || rtag == AST_FIELD_REF)
  {
    op3 = Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
    insSet->add_instruction(new Instruction(HINS_LOAD_INT, op3, op2));
    op2 = op3;
  }

  // add compare instruction
  insSet->add_instruction(new Instruction(HINS_INT_COMPARE, op1, op2));

  // add jump instruction
  switch (tag)
  {
  case AST_COMPARE_EQ:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JNE, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JE, *label));
    break;
  case AST_COMPARE_NEQ:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JE, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JNE, *label));
    break;
  case AST_COMPARE_LT:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JGTE, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JLT, *label));
    break;
  case AST_COMPARE_LTE:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JGT, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JLTE, *label));

    break;
  case AST_COMPARE_GT:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JLTE, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JGT, *label));

    break;
  case AST_COMPARE_GTE:
    if (isReverse)
      insSet->add_instruction(new Instruction(HINS_JLT, *label));
    else
      insSet->add_instruction(new Instruction(HINS_JGTE, *label));
    break;

  default:
    cerr << "Error: Undefined compare operator" << endl;
    exit(-1);
  }

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);

}

void InsGenerator::visit_write(struct Node *ast)
{
  recur_on_children(ast); // default behavior

  struct Node *childNode = node_get_kid(ast, 0);
  int tag = node_get_tag(childNode);

  Operand childOperand = *(childNode->getOperand());
  Instruction *ins;

  if (tag == AST_ARRAY_ELEMENT_REF || tag == AST_FIELD_REF)
  {
    // child operand is address
    Operand op1 = Operand(OPERAND_VREG, childOperand.get_base_reg() + 1);
    // load value from address
    ins = new Instruction(HINS_LOAD_INT, op1, childOperand);
    insSet->add_instruction(ins);
    ins = new Instruction(HINS_WRITE_INT, op1);
  }
  else
  {
    // child operand is value
    ins = new Instruction(HINS_WRITE_INT, childOperand);
  }

  // write vr
  insSet->add_instruction(ins);

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);

}

void InsGenerator::visit_read(struct Node *ast)
{


  recur_on_children(ast);
  struct Node *childNode = node_get_kid(ast, 0);
  int tag = node_get_tag(childNode);

  Operand childOperand = *(childNode->getOperand());
  Instruction *ins;

  int rIdx = insVRIdx;

  Operand op1;
  // read value into a new vr
  op1 = Operand(OPERAND_VREG, rIdx);
  ins = new Instruction(HINS_READ_INT, op1);
  insSet->add_instruction(ins);

  // move value to correspond vr or address
  if (tag == NODE_TOK_IDENT)
  {
    // child operand is value
    ins = new Instruction(HINS_MOV, childOperand, op1);
  }
  else if (tag == AST_ARRAY_ELEMENT_REF || tag == AST_FIELD_REF)
  {
    // child operand is address
    ins = new Instruction(HINS_STORE_INT, childOperand, op1);
  }

  insSet->add_instruction(ins);

  // deal with myTable->vrIdxMax
  myTable->vrIdxMax = max(myTable->vrIdxMax, insVRIdx);

  //Edit
  insVRIdx++;

}

Type *InsGenerator::visit_var_ref(struct Node *ast)
{
  // recur_on_children(ast); // default behavior
  int tag = node_get_tag(ast);
  switch (tag)
  {

  case AST_ARRAY_ELEMENT_REF:
    return visit_array_element_ref(ast);
  case AST_FIELD_REF:
    return visit_field_ref(ast);
  case NODE_TOK_IDENT:
    return visit_identifier(ast);
  default:
    cerr << "Error: Not a reference" << endl;
    exit(-1);
  }
}

Type *InsGenerator::visit_array_element_ref(struct Node *ast)
{

  // load basic information
  struct Node *array = node_get_kid(ast, 0);
  struct Node *index = node_get_kid(ast, 1);

  const char *lexeme = node_get_str(array);

  lexeme = node_get_str(index);
  string idxS(lexeme);
  int idx = -1;

  int idxTag = node_get_tag(index);

  if (idxTag == AST_INT_LITERAL)
  {
    idx = stoi(idxS);
  }
  Type *arraysType = visit_var_ref(array);
  Type *elementType = arraysType->arrayType; // elementType

  // instruction prepare;
  Instruction *ins;

  Operand op1, op2, op3;

  if (idxTag == AST_INT_LITERAL)
  {

    // instruction for loading index
    op1 = Operand(OPERAND_VREG, insVRIdx);
    op2 = Operand(OPERAND_INT_LITERAL, idx);
    ins = new Instruction(HINS_LOAD_ICONST, op1, op2);
    insVRIdx++;
    insSet->add_instruction(ins);
  }

  int elementSize = elementType->size;

  // calculate offset

  if (idxTag == AST_INT_LITERAL)
  {
    op2 = Operand(OPERAND_VREG, insVRIdx - 1);
  }
  else if (idxTag == NODE_TOK_IDENT)
  {
    // get the index from register
    Symbol *idxSymbol = myTable->lookup(idxS);
    op2 = *(idxSymbol->getOperand());
  }
  else
  {
    visitOperator(index);
    op2 = *(index->getOperand());
  }

  op1 = Operand(OPERAND_VREG, insVRIdx);

  op3 = Operand(OPERAND_INT_LITERAL, elementSize);
  ins = new Instruction(HINS_INT_MUL, op1, op2, op3);
  insVRIdx++;
  insSet->add_instruction(ins);

  // calculate address
  op1 = Operand(OPERAND_VREG, insVRIdx);
  if (idxTag == AST_INT_LITERAL)
  {
    op2 = Operand(OPERAND_VREG, insVRIdx - 3);
  }
  else if (idxTag == NODE_TOK_IDENT)
  {
    op2 = Operand(OPERAND_VREG, insVRIdx - 2);
  }
  else
  {
    op2 = *(array->getOperand());
  }

  op3 = Operand(OPERAND_VREG, insVRIdx - 1);
  ins = new Instruction(HINS_INT_ADD, op1, op2, op3);
  insVRIdx++;
  insSet->add_instruction(ins);

  Operand *curOperand = new Operand(OPERAND_VREG, insVRIdx - 1);
  ast->setOperand(curOperand);

  return elementType;
}

Type *InsGenerator::visit_field_ref(struct Node *ast)
{
  struct Node *record = node_get_kid(ast, 0);
  struct Node *field = node_get_kid(ast, 1);
  int rtag = node_get_tag(record);
  Operand *op1;
  Operand op2, op3;

  Type *recordType = visit_var_ref(record);
  ;

  // get record's symbol table
  SymbolTable *curTable = recordType->recordTable;
  Type *fieldType;

  // op1: operand to store the address of field reference
  op1 = new Operand(OPERAND_VREG, insVRIdx);
  insVRIdx++;
  // op2: base address of record
  op2 = *(record->getOperand());
  // op3: offset
  const char *lexeme = node_get_str(field);
  string fieldName(lexeme);
  Symbol *fieldSymbol = curTable->lookup(lexeme);
  Operand *fieldOperand = fieldSymbol->getOperand();
  fieldType = fieldSymbol->get_type();

  int insideVRIdx = fieldOperand->get_base_reg();
  int offset;

  if (fieldType->kind == ARRAY || fieldType->kind == RECORD)
  {
    // address for array or record
    offset = insideVRIdx;
  }
  else
  {
    // address for name variable
    offset = curTable->structSize + insideVRIdx * 8;
  }
  op3 = Operand(OPERAND_INT_LITERAL, offset);

  insSet->add_instruction(new Instruction(HINS_INT_ADD, *op1, op2, op3));
  ast->setOperand(op1);

  // deal with elementType
  return fieldType;
}

Type *InsGenerator::visit_identifier(struct Node *ast)
{

  const char *lexeme = node_get_str(ast);
  string varName(lexeme);

  Symbol *curSymbol = myTable->lookup(varName);

  if (curSymbol->get_type()->kind == RECORD || curSymbol->get_type()->kind == ARRAY)
  { // add array case !!
    // if the identifier is a record
    int address = curSymbol->getOperand()->get_base_reg();
    Operand op2;
    op2 = Operand(OPERAND_INT_LITERAL, address);
    Operand *op1 = new Operand(OPERAND_VREG, insVRIdx);
    insVRIdx++;
    insSet->add_instruction(new Instruction(HINS_LOCALADDR, *op1, op2));
    ast->setOperand(op1);
  }
  else
  {
    // the identifier is a normal value
    ast->setOperand(curSymbol->getOperand());
  }

  // deal with elementType
  return curSymbol->get_type();
}

void InsGenerator::recur_on_children(struct Node *ast)
{
  int num_kids = node_get_num_kids(ast);
  for (int i = 0; i < num_kids; i++)
  {
    visit(node_get_kid(ast, i));
  }
}
