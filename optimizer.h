#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <cassert>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "highlevel.h"

class Optimizer{

    public:
    void visitCfg(ControlFlowGraph* input,ControlFlowGraph* output);
    virtual void visitBlock(BasicBlock* inputBlock);
    virtual void visitIns(Instruction* curIns);
    BasicBlock* findBlock(ControlFlowGraph* cfg,unsigned int bbIdx);

    BasicBlock* curBlock;
    ControlFlowGraph* src;
    ControlFlowGraph* dst;

    Optimizer();
    void optimize();

}; 

struct VNKey {
    public:
    int opcode;
    int vn1;
    int vn2;

    VNKey(int input1,int input2, int input3);
};

class ValueNumbering :public Optimizer{
    private:
    unordered_map<int,int> constToVn ;
    unordered_map<int,int> vnToConst ;
    unordered_map<int,int> vrToVn ;
    unordered_map<int,int> vnToVr ;
    unordered_map<VNKey*,int> keyToVn;

    int nextVN;

    //ControlFlowGraph* src;
    //visitCfg(ControlFlowGraph* src);
    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    void handleMov(Instruction* curIns);
    void handleExp(Instruction* curIns);

    //BasicBlock* findBlock(ControlFlowGraph* cfg,unsigned int bbIdx);


    public:
    //ControlFlowGraph* dst;
    //ValueNumbering();
    ValueNumbering(ControlFlowGraph* input);
    //void optimize();

}; 

class ConstProp :public Optimizer{
    private:
    unordered_map<int,int> vrToConst;

    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    public:
    ConstProp(ControlFlowGraph* input);

}; 


#endif // OPTIMIZER_H