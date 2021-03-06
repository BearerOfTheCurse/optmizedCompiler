#ifndef AST_H
#define AST_H

#ifdef __cplusplus
extern "C" {
#endif

enum ASTKind {
  AST_PROGRAM = 10000,
  AST_DECLARATIONS,
  AST_CONSTANT_DECLARATIONS,
  AST_CONSTANT_DEF,
  AST_TYPE_DECLARATIONS,
  AST_TYPE_DEF,
  AST_NAMED_TYPE,
  AST_ARRAY_TYPE,
  AST_RECORD_TYPE,
  AST_VAR_DECLARATIONS,
  AST_VAR_DEF,
  AST_ADD,
  AST_SUBTRACT,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_MODULUS,
  AST_NEGATE,
  AST_INT_LITERAL,

  AST_INSTRUCTIONS,

  AST_ASSIGN,
  AST_IF,
  AST_IF_ELSE,
  AST_REPEAT,
  AST_WHILE,

  AST_COMPARE_EQ,
  AST_COMPARE_NEQ,
  AST_COMPARE_LT,
  AST_COMPARE_LTE,
  AST_COMPARE_GT,
  AST_COMPARE_GTE,

  AST_WRITE,
  AST_READ,

  AST_VAR_REF,
  AST_ARRAY_ELEMENT_REF,
  AST_FIELD_REF,

  AST_IDENTIFIER_LIST,
  AST_EXPRESSION_LIST,
};

const char *ast_get_tag_name(int ast_tag);

struct Node;

void ast_print_graph(struct Node *ast);
const void extractChild(struct Node* source, struct Node* target); 
const void makeChild(struct Node* source, struct Node* target);  
struct Node* dealPrimary(struct Node* input);

#ifdef __cplusplus
}
#endif

#endif // AST_H
