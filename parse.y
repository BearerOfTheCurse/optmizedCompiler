%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "grammar_symbols.h"
#include "util.h"
#include "ast.h"
#include "node.h"

struct Node *g_program;

void yyerror(const char *fmt, ...);

int yylex(void);
%}

%union {
  struct Node *node;
}

%token<node> TOK_IDENT TOK_INT_LITERAL

%token<node> TOK_PROGRAM TOK_BEGIN TOK_END TOK_CONST TOK_TYPE TOK_VAR
%token<node> TOK_ARRAY TOK_OF TOK_RECORD TOK_DIV TOK_MOD TOK_IF
%token<node> TOK_THEN TOK_ELSE TOK_REPEAT TOK_UNTIL TOK_WHILE TOK_DO
%token<node> TOK_READ TOK_WRITE

%token<node> TOK_ASSIGN
%token<node> TOK_SEMICOLON TOK_EQUALS TOK_COLON TOK_PLUS TOK_MINUS TOK_TIMES
%token<node> TOK_HASH TOK_LT TOK_GT TOK_LTE TOK_GTE TOK_LPAREN
%token<node> TOK_RPAREN TOK_LBRACKET TOK_RBRACKET TOK_DOT TOK_COMMA

%type<node> program

%type<node> opt_declarations declarations declaration constdecl constdefn_list constdefn
%type<node> typedecl typedefn_list typedefn vardecl type vardefn_list vardefn
%type<node> expression term factor unary primary 
%type<node> opt_instructions instructions instruction
%type<node> assignstmt ifstmt repeatstmt whilestmt condition writestmt readstmt
%type<node> designator identifier_list expression_list

%%

/* TODO: add grammar productions */

program
    /* FIXME: this is not the complete/correct production for program */
  : TOK_PROGRAM TOK_IDENT TOK_SEMICOLON opt_declarations TOK_BEGIN opt_instructions TOK_END TOK_DOT { g_program = $$ = node_build2(AST_PROGRAM,$4,$6);}
  ;

opt_declarations 
  : declaration opt_declarations {$$ = $1; extractChild($2,$$);}
  | /* epsilon */ {$$ = node_build0(AST_DECLARATIONS);}
  ;

opt_instructions 
  : instruction TOK_SEMICOLON opt_instructions {$$ = node_build1(AST_INSTRUCTIONS,$1); extractChild($3,$$);} 
  | /* epsilon */ {$$ = node_build0(AST_INSTRUCTIONS);}
  ;

declaration 
  :TOK_CONST constdefn_list {$$ = node_build0(AST_DECLARATIONS); extractChild($2,$$);}
  |TOK_VAR vardefn_list {$$ = node_build0(AST_DECLARATIONS); extractChild($2,$$);}
  |TOK_TYPE typedefn_list {$$ = node_build0(AST_DECLARATIONS); extractChild($2,$$);}
  ;
  

constdefn_list  
  :constdefn  {$$ = node_build1(AST_CONSTANT_DECLARATIONS,$1);}
  |constdefn constdefn_list {$$ = node_build1(AST_CONSTANT_DECLARATIONS,$1); extractChild($2,$$);}
  ;

constdefn
  :TOK_IDENT TOK_EQUALS expression TOK_SEMICOLON {$$ = node_build2(AST_CONSTANT_DEF,$1,$3);}
  ;

vardefn_list 
  :vardefn {$$ = node_build1(AST_VAR_DECLARATIONS,$1);}
  |vardefn vardefn_list {$$ = node_build1(AST_VAR_DECLARATIONS,$1); extractChild($2,$$);}
  ;

vardefn 
  :identifier_list TOK_COLON type TOK_SEMICOLON {$$ = node_build1(AST_VAR_DEF,$3); makeChild($1,$$);}
  ;

identifier_list 
  :TOK_IDENT {$$ = $1;}
  |TOK_IDENT TOK_COMMA identifier_list {$$ = node_build1(AST_IDENTIFIER_LIST,$1); makeChild($3,$$);}
  ;

type
  :TOK_IDENT {$$ = node_build1(AST_NAMED_TYPE,$1);}
  |TOK_ARRAY expression TOK_OF type {$$ = node_build2(AST_ARRAY_TYPE,$2,$4);}
  |TOK_RECORD vardefn_list TOK_END {$$ = node_build0(AST_RECORD_TYPE); makeChild($2,$$);}
  ;

typedefn_list
  :typedefn {$$ = node_build1(AST_TYPE_DECLARATIONS,$1);}
  |typedefn typedefn_list {$$ = node_build1(AST_TYPE_DECLARATIONS,$1); extractChild($2,$$);}
  ;

typedefn 
  :TOK_IDENT TOK_EQUALS type TOK_SEMICOLON {$$ = node_build2(AST_TYPE_DEF,$1,$3);}


expression
  : term { $$ = $1; }
  ;

term 
  : term TOK_PLUS factor {$$ = node_build2(AST_ADD,$1,$3);}
  | term TOK_MINUS factor {$$ = node_build2(AST_SUBTRACT,$1,$3);}
  | factor {$$ = $1;} 
  ;

factor
  : factor TOK_TIMES unary {$$ = node_build2(AST_MULTIPLY,$1,$3);}
  | factor TOK_DIV unary   {$$ = node_build2(AST_DIVIDE,$1,$3);}
  | factor TOK_MOD unary   {$$ = node_build2(AST_MODULUS,$1,$3);}
  | unary{$$ = $1;} 
  ;

unary
  : TOK_PLUS unary {$$ = node_build2(AST_NEGATE,$1,$2); }
  | TOK_MINUS unary {$$ = node_build2(AST_NEGATE,$1,$2); }
  | primary {$$ = $1;} 

primary
  : TOK_INT_LITERAL { $$ = dealPrimary($1);}
  | designator {$$ = $1;}
  | TOK_LPAREN expression TOK_RPAREN {$$ = $2;} 
  ;

instruction
  : designator TOK_ASSIGN expression {$$ = node_build2(AST_ASSIGN,$1,$3);}
  |TOK_IF condition TOK_THEN opt_instructions TOK_END {$$ = node_build2(AST_IF,$2,$4);}
  |TOK_IF condition TOK_THEN opt_instructions TOK_ELSE opt_instructions TOK_END {$$ = node_build3(AST_IF_ELSE,$2,$4,$6);}
  |TOK_REPEAT opt_instructions TOK_UNTIL condition TOK_END {$$ = node_build2(AST_REPEAT,$2,$4);}
  |TOK_WHILE condition TOK_DO opt_instructions TOK_END {$$ = node_build2(AST_WHILE,$2,$4);}
  |TOK_WRITE expression {$$ = node_build1(AST_WRITE,$2);}
  |TOK_READ designator {$$ = node_build1(AST_READ,$2);}
  ;

designator
  : TOK_IDENT { $$ = $1 ;}
  | designator TOK_LBRACKET expression_list TOK_RBRACKET {$$ = node_build2(AST_ARRAY_ELEMENT_REF,$1,$3);}
  | designator TOK_DOT TOK_IDENT {$$ = node_build2(AST_FIELD_REF,$1,$3);} 
  ;

expression_list
  : expression {$$ = $1;}
  | expression TOK_COMMA expression_list{$$ = node_build1(AST_EXPRESSION_LIST,$1); makeChild($2,$$);}
  ;

condition 
  : expression TOK_GT expression {$$ = node_build2(AST_COMPARE_GT,$1,$3);}
  | expression TOK_GTE expression {$$ = node_build2(AST_COMPARE_GTE,$1,$3);}
  | expression TOK_LT expression {$$ = node_build2(AST_COMPARE_LT,$1,$3);}
  | expression TOK_LTE expression {$$ = node_build2(AST_COMPARE_LTE,$1,$3);}
  | expression TOK_EQUALS expression {$$ = node_build2(AST_COMPARE_EQ,$1,$3);}
  | expression TOK_HASH expression {$$ = node_build2(AST_COMPARE_NEQ,$1,$3);}
  ;




%%

void yyerror(const char *fmt, ...) {
  extern char *g_srcfile;
  extern int yylineno, g_col;

  va_list args;

  va_start(args, fmt);
  fprintf(stderr, "%s:%d:%d: Error: ", g_srcfile, yylineno, g_col);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);

  exit(1);
}
