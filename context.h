#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

// Context gathers all of the data structures needed for
// semantic analysis, code generation, and code optimization.

struct Node;
struct Context;

struct Context *context_create(struct Node *ast);
void context_destroy(struct Context *ctx);

// This function can be called multiple times to configure
// compilation options.  Flags available:
//   's' - print symbol table info
void context_set_flag(struct Context *ctx, char flag);

void context_build_symtab(struct Context *ctx);
void context_check_types(struct Context *ctx);

void context_build_IR(struct Context *ctx);
void context_print_IR(struct Context *ctx);

void context_build_code(struct Context *ctx);
void context_print_code(struct Context *ctx);

void context_build_good_code(struct Context *ctx);
void context_print_good_code(struct Context *ctx);


void context_build_high_cfg(struct Context *ctx);
void context_print_high_cfg(struct Context *ctx);

void context_build_x86_cfg(struct Context *ctx);
void context_print_x86_cfg(struct Context *ctx);

#ifdef __cplusplus
}
#endif

#endif // CONTEXT_H
