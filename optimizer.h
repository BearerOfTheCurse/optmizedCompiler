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
#include "live_vregs.h"

class Optimizer{

    public:
    void visitCfg(ControlFlowGraph* input,ControlFlowGraph* output);
    void resetCfg(ControlFlowGraph* input);
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
    // order
    // memory
    // peehole

    private:
    unordered_map<int,int> constToVn ;
    unordered_map<int,int> vnToConst ;
    unordered_map<int,int> vrToVn ;
    unordered_map<int,int> vnToVr ;
    unordered_map<VNKey*,int> keyToVn;

    int nextVN;
    int memOffset;

    //ControlFlowGraph* src;
    //visitCfg(ControlFlowGraph* src);
    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    void handleMov(Instruction* curIns);
    void handleExp(Instruction* curIns);
    void handleMemory(Instruction* curIns);
    void handleRead(Instruction* curIns);

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

class VRProp :public Optimizer{
    private:
    unordered_map<int,int> vrToVr;

    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    public:
    VRProp(ControlFlowGraph* input);

}; 

class Peephole:public Optimizer{
    private:

    unordered_map<int,int> vrToVr;

    void replace(Instruction* ins);
    bool ldiMovRule(BasicBlock* inputBlock, int insIdx);  // ldi v3 (v4); mov v5 v3; => ldi v5 (v4)

    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    public:
    Peephole(ControlFlowGraph* input);

}; 

class Cleaner:public Optimizer{
    private:
    LiveVregs* analyzer;

    void visitBlock(BasicBlock* curBlock);
    void visitIns(Instruction* curIns);

    public:
    Cleaner(ControlFlowGraph* input);

}; 


#endif // OPTIMIZER_H