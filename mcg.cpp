#include <string>
#include <unordered_map>
#include <iostream>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "mcg.h"
#include "highlevel.h"
#include "x86_64.h"

using namespace std;

Mcg::Mcg(SymbolTable *table, ControlFlowGraph *inGraph) : myTable(table), highCfg(inGraph)
{
    outSeq = new InstructionSequence();
    lowCfg = new ControlFlowGraph();

    rsp = Operand(OPERAND_MREG, MREG_RSP);
    rbp = Operand(OPERAND_MREG, MREG_RBP);
    rdi = Operand(OPERAND_MREG, MREG_RDI);
    rsi = Operand(OPERAND_MREG, MREG_RSI);

    rax = Operand(OPERAND_MREG, MREG_RAX);
    rbx = Operand(OPERAND_MREG, MREG_RBX);
    rcx = Operand(OPERAND_MREG, MREG_RCX);
    rdx = Operand(OPERAND_MREG, MREG_RDX);

    r8 = Operand(OPERAND_MREG, MREG_R8);
    r9 = Operand(OPERAND_MREG, MREG_R9);
    r10 = Operand(OPERAND_MREG, MREG_R10);
    r11 = Operand(OPERAND_MREG, MREG_R11);
    r12 = Operand(OPERAND_MREG, MREG_R12);
    r13 = Operand(OPERAND_MREG, MREG_R13);
    r14 = Operand(OPERAND_MREG, MREG_R14);
    r15 = Operand(OPERAND_MREG, MREG_R15);

    readCommand = Operand("s_readint_fmt", true);
    scanf_label = Operand("scanf");

    //building mr(the mapping)

    Operand* op;
    op = new Operand();  *op =  rbx; mr[0] = op;
    op = new Operand();  *op =  r12;  mr[1] = op;
    op = new Operand();  *op =  r13;  mr[2] = op;
    op = new Operand();  *op =  r14;  mr[3] = op;
    op = new Operand();  *op =  r15;  mr[4] = op;
//callee
    op = new Operand();  *op =  r8;  mr[5] = op;
    op = new Operand();  *op =  r9;  mr[6] = op;
    op = new Operand();  *op =  r10;  mr[7] = op;
    op = new Operand();  *op =  r11;  mr[8] = op;

    for(int i=0;i<maxMr;i++){
        mrToVr[i] = -1;
    }


}

void Mcg::oneStatement(Instruction *ins)
{
    int op_code = ins->get_opcode();
    switch (op_code)
    {

    case HINS_NOP:
        nop(ins);
        break;
    case HINS_LOAD_ICONST:
        load_iconst(ins);
        break;
    case HINS_INT_ADD:
        add(ins);
        break;
    case HINS_INT_SUB:
        sub(ins);
        break;
    case HINS_INT_MUL:
        mul(ins);
        break;
    case HINS_INT_DIV:
        div(ins);
        break;
    case HINS_INT_MOD:
        mod(ins);
        break;
    case HINS_INT_NEGATE:
        negate(ins);
        break;
    case HINS_LOCALADDR:
        localAddr(ins);
        break;
    case HINS_LOAD_INT:
        loadInt(ins);
        break;
    case HINS_STORE_INT:
        storeInt(ins);
        break;
    case HINS_READ_INT:
        readi(ins);
        break;
    case HINS_WRITE_INT:
        writei(ins);
        break;
    case HINS_JUMP:
        jmp(ins);
        break;
    case HINS_JE:
        je(ins);
        break;
    case HINS_JNE:
        jne(ins);
        break;
    case HINS_JLT:
        jlt(ins);
        break;
    case HINS_JLTE:
        jlte(ins);
        break;
    case HINS_JGT:
        jgt(ins);
        break;
    case HINS_JGTE:
        jgte(ins);
        break;
    case HINS_INT_COMPARE:
        compare(ins);
        break;
    case HINS_MOV:
        mov(ins);
        break;

    default:
        cerr << "ERROR: Unknown high level code type" << endl;
        exit(-1);
    }
}

BasicBlock* Mcg::findBlock(ControlFlowGraph* cfg,unsigned int bbIdx){
  for (auto i =lowCfg->bb_begin(); i != lowCfg->bb_end(); i++) {
      if( (*i)->get_id() == bbIdx) return (*i);
  }
  cerr<<"Error: Cannot find the block with idx: "<<bbIdx<<endl;
  exit(-1);
}

void Mcg::visitCfg()
{
    vector<Edge*> myEdgeList;
    //deal with block
  for (auto i = highCfg->bb_begin(); i != highCfg->bb_end(); i++) {
    BasicBlock *bb = *i;
    BasicBlockKind bb_kind = bb->get_kind();


     if (bb->has_label()) {
         curBlock= lowCfg->create_basic_block(bb->get_kind(),bb->get_label());
     }else{
         curBlock= lowCfg->create_basic_block(bb->get_kind(),"");
     }

    if (bb_kind == BASICBLOCK_INTERIOR) {
        visitBlock(bb);
    }
  }


  //deal with edge
  BasicBlock* target;
  BasicBlock* newSource;
  BasicBlock* newTarget;

  for (auto i = highCfg->bb_begin(); i != highCfg->bb_end(); i++) {
    BasicBlock *bb = *i;
    myEdgeList = highCfg->get_outgoing_edges(bb);

    for(int j=0;j<myEdgeList.size();j++){
        target = myEdgeList[j]->get_target();
        newSource = findBlock(lowCfg,bb->get_id());
        newTarget= findBlock(lowCfg,target->get_id());
        lowCfg->create_edge(newSource,newTarget,myEdgeList[j]->get_kind());
    }
  }

  outSeq = lowCfg->create_instruction_sequence();

}

void Mcg::visitBlock(BasicBlock* curBlock)
{

    // begin going through
    int sz = curBlock->get_length();
    Instruction *ins;
    string label;

// update liveTime for this block
    liveTime.clear();
    updateLiveTime(curBlock);
    unordered_map<int,int>:: iterator it;

// update vrToMr and mrToVr for this block
    it = mrToVr.begin();
    while(it != mrToVr.end()){
        if(liveTime.count(it->second) == 0 && it->second >= myTable->vrIdx ){
            // delete original record
            vrToMr.erase(it->second);
            mrToVr[it->first]=-1;
        }
        it++;
    }

    for (int i = 0; i < sz; i++)
    {
        insIdx = i+1;
        ins = curBlock->get_instruction(i);
        oneStatement(ins);
    }

    // if these is a label at the end of the code
}

Operand* Mcg::getOperand(int vrIdx,bool use){
    if(vrIdx < myTable->vrIdx){
        int offset = vrIdx * 8 + myTable->structSize;
        return new Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
    }else{
        if(use) return getMrUse(vrIdx);
        else return getMrStore(vrIdx);
    }
}

void Mcg::nop(Instruction *ins)
{
    curBlock->add_instruction(new Instruction(MINS_NOP));
}

void Mcg::load_iconst(Instruction *ins)
{
    int vrIdx = ins->get_operand(0).get_base_reg();  
    Operand dst = *(getOperand(vrIdx,0));

    int val = ins->get_operand(1).get_int_value();
    Operand src(OPERAND_INT_LITERAL, val);

    Instruction *first = new Instruction(MINS_MOVQ, src, dst);
    first->set_comment("ldci");   // add a comment for easier checking
    curBlock->add_instruction(first);

}

void Mcg::add(Instruction *ins)   
{


    int lOffset, rOffset; 
    Operand leftOp, rightOp;
    int opIdx;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
       opIdx = ins->get_operand(1).get_base_reg();
       leftOp = *(getOperand(opIdx,1));

    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
       opIdx = ins->get_operand(2).get_base_reg();
       rightOp = *(getOperand(opIdx,1));
    }


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des = *(getOperand(opIdx,0));

    Instruction *first = new Instruction(MINS_MOVQ, leftOp, des);
    first->set_comment("add");  // add command for easier checking

    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_ADDQ, rightOp, des));
}
void Mcg::sub(Instruction *ins)
{

    int lOffset, rOffset; 
    Operand leftOp, rightOp;
    int opIdx;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
       opIdx = ins->get_operand(1).get_base_reg();
       leftOp = *(getOperand(opIdx,1));
    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
       opIdx = ins->get_operand(2).get_base_reg();
       rightOp = *(getOperand(opIdx,1));
    }


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des = *(getOperand(opIdx,0));
    Instruction *first = new Instruction(MINS_MOVQ, leftOp, des);
    first->set_comment("sub");
    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_SUBQ, rightOp, des));

}
void Mcg::mul(Instruction *ins)
{


    int lOffset, rOffset; 
    Operand leftOp, rightOp;
    int opIdx;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
       opIdx = ins->get_operand(1).get_base_reg();
           leftOp = *(getOperand(opIdx,1));

    }

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
       opIdx = ins->get_operand(2).get_base_reg();
           rightOp = *(getOperand(opIdx,1));
    }


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des = *(getOperand(opIdx,0));

    
    Instruction *first = new Instruction(MINS_MOVQ, leftOp, des);
    first->set_comment("mul");
    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_IMULQ, rightOp, des));

}
void Mcg::div(Instruction *ins)
{
    int opIdx,lOffset,rOffset;
    Operand leftOp,rightOp;


    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        opIdx = ins->get_operand(1).get_base_reg();
        leftOp = *(getOperand(opIdx,1));

    }



    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        opIdx = ins->get_operand(2).get_base_reg();
        rightOp = *(getOperand(opIdx,1));
    }


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des= *(getOperand(opIdx,0));


    Instruction *first = new Instruction(MINS_MOVQ, leftOp, rax);
    first->set_comment("div");

    curBlock->add_instruction(first);

    storeCallerMr();
    curBlock->add_instruction(new Instruction(MINS_CQTO));

    loadCallerMr();


    //If the rightop is an constant, the assembly code cannot work
    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL){
        curBlock->add_instruction(new Instruction(MINS_MOVQ, rightOp,rdi));
        curBlock->add_instruction(new Instruction(MINS_IDIVQ, rdi));
    }else{
        curBlock->add_instruction(new Instruction(MINS_IDIVQ, rightOp));
    }

    curBlock->add_instruction(new Instruction(MINS_MOVQ, rax, des));
}
void Mcg::mod(Instruction *ins)
{
    int opIdx;
    Operand leftOp, rightOp;


    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        opIdx= ins->get_operand(1).get_int_value();
        leftOp = Operand(OPERAND_INT_LITERAL, opIdx);
    }
    else
    {
        opIdx = ins->get_operand(1).get_base_reg();
        leftOp = *(getOperand(opIdx,1));

    }


    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL)
    {
        opIdx = ins->get_operand(2).get_int_value();
        rightOp = Operand(OPERAND_INT_LITERAL, opIdx);
    }
    else
    {
        opIdx = ins->get_operand(2).get_base_reg();
        rightOp = *(getOperand(opIdx,1));
    }


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des= *(getOperand(opIdx,0));


    Instruction *first = new Instruction(MINS_MOVQ, leftOp, rax);
    first->set_comment("mod");

    curBlock->add_instruction(first);

    storeCallerMr();
    curBlock->add_instruction(new Instruction(MINS_CQTO));

    loadCallerMr();

    if (ins->get_operand(2).get_kind() == OPERAND_INT_LITERAL){
        curBlock->add_instruction(new Instruction(MINS_MOVQ, rightOp,rdi));
        curBlock->add_instruction(new Instruction(MINS_IDIVQ, rdi));
    }else{
        curBlock->add_instruction(new Instruction(MINS_IDIVQ, rightOp));
    }
    curBlock->add_instruction(new Instruction(MINS_MOVQ, rdx, des));
}
void Mcg::negate(Instruction *ins)
{
}

void Mcg::localAddr(Instruction *ins)
{
    int offset1 = ins->get_operand(1).get_int_value();
    Operand address(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset1);

    int opIdx = ins->get_operand(0).get_base_reg();
       Operand localvar = *(getOperand(opIdx,0));

    Instruction *first = new Instruction(MINS_LEAQ, address, localvar);
    first->set_comment("localaddr");

    curBlock->add_instruction(first);
}
void Mcg::loadInt(Instruction *ins)
{

    int opIdx;
    Operand left,right;

    opIdx = ins->get_operand(0).get_base_reg();
    left = *(getOperand(opIdx,0));

    opIdx = ins->get_operand(1).get_base_reg();
    right = *(getOperand(opIdx,1));

    Operand tmp(OPERAND_MREG_MEMREF,right.get_base_reg());

    Instruction *first = new Instruction(MINS_MOVQ, tmp, left);
    first->set_comment("ldi");
    curBlock->add_instruction(first);

}
void Mcg::storeInt(Instruction *ins)
{
    int lOffset, rOffset,opIdx;
    Operand left,right;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(0).get_int_value();
        left = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
        opIdx = ins->get_operand(0).get_base_reg();
        left = *(getOperand(opIdx,1));
    }

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(1).get_int_value();
        right = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
        opIdx = ins->get_operand(1).get_base_reg();
        right = *(getOperand(opIdx,1));
    }

     Instruction *first = new Instruction(MINS_MOVQ, right, rsi);
     first->set_comment("sti");

     curBlock->add_instruction(first);

    Operand tmp(OPERAND_MREG_MEMREF,left.get_base_reg());
    curBlock->add_instruction(new Instruction(MINS_MOVQ, rsi, tmp));


}

void Mcg::readi(Instruction *ins)       
{
    Operand localvar;
    int offset;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        offset = ins->get_operand(0).get_int_value();
        localvar = Operand(OPERAND_INT_LITERAL, offset);
    }
    else
    {
       offset = ins->get_operand(0).get_base_reg() * 8 + myTable->structSize;
       localvar = Operand(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, offset);
    }

    int opIdx = ins->get_operand(0).get_base_reg();
    Operand mrOperand = *(getMrStore(opIdx));

    Instruction *first = new Instruction(MINS_MOVQ, readCommand, rdi);
    first->set_comment("readi");

    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_LEAQ, localvar, rsi));

    storeCallerMr();  //keep caller register's value
    curBlock->add_instruction(new Instruction(MINS_CALL, scanf_label));

    loadCallerMr();
    curBlock->add_instruction(new Instruction(MINS_MOVQ, localvar, mrOperand));

}

void Mcg::writei(Instruction *ins)
{
    Operand writeCommand("s_writeint_fmt", true);
    Operand printf_label("printf");

    Operand localvar;
    int offset;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        offset = ins->get_operand(0).get_int_value();
        localvar = Operand(OPERAND_INT_LITERAL, offset);
    }
    else
    {
       int opIdx = ins->get_operand(0).get_base_reg();
       localvar = *(getOperand(opIdx,1));
    }

    Instruction *first = new Instruction(MINS_MOVQ, localvar, rsi);
    first->set_comment("writei");


    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_MOVQ, writeCommand, rdi));
    storeCallerMr();        //keep caller register's value
    curBlock->add_instruction(new Instruction(MINS_CALL, printf_label));
    loadCallerMr();         
}

void Mcg::jmp(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JMP, op));
}

void Mcg::je(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JE, op));
}
void Mcg::jne(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JNE, op));
}
void Mcg::jlt(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JL, op));
}

void Mcg::jlte(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JLE, op));
}

void Mcg::jgt(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JG, op));
}

void Mcg::jgte(Instruction *ins)
{
    Operand op = ins->get_operand(0);
    curBlock->add_instruction(new Instruction(MINS_JGE, op));
}

void Mcg::compare(Instruction *ins)
{

    int lOffset, rOffset,opIdx;
    Operand left, right;

    if (ins->get_operand(0).get_kind() == OPERAND_INT_LITERAL)
    {
        lOffset = ins->get_operand(0).get_int_value();
        left = Operand(OPERAND_INT_LITERAL, lOffset);
    }
    else
    {
       opIdx = ins->get_operand(0).get_base_reg();
       left = *(getOperand(opIdx,1));
    }

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        rOffset = ins->get_operand(1).get_int_value();
        right = Operand(OPERAND_INT_LITERAL, rOffset);
    }
    else
    {
       opIdx = ins->get_operand(1).get_base_reg();
       right = *(getOperand(opIdx,1));
    }

    Instruction *first = new Instruction(MINS_MOVQ, left, rdx);
    first->set_comment("compare");

    curBlock->add_instruction(first);
    curBlock->add_instruction(new Instruction(MINS_CMPQ, right, rdx));
}
void Mcg::mov(Instruction *ins)
{
    int sOffset; 
    Operand src;
    int opIdx;

    if (ins->get_operand(1).get_kind() == OPERAND_INT_LITERAL)
    {
        sOffset = ins->get_operand(1).get_int_value();
        src = Operand(OPERAND_INT_LITERAL, sOffset);
    }
    else
    {
       opIdx = ins->get_operand(1).get_base_reg();
       src = *(getOperand(opIdx,1));
    }
    


    opIdx = ins->get_operand(0).get_base_reg();
    Operand des= *(getOperand(opIdx,0));

    if(opIdx < myTable->vrIdx 
        && ins->get_operand(1).has_base_reg() 
        && ins->get_operand(1).get_base_reg() < myTable->vrIdx){

        Instruction *first = new Instruction(MINS_MOVQ, src, rdi);
        first->set_comment("mov");

        curBlock->add_instruction(first);
        curBlock->add_instruction(new Instruction(MINS_MOVQ, rdi, des));

        }else{

        Instruction *first = new Instruction(MINS_MOVQ, src, des);
        first->set_comment("mov");
        curBlock->add_instruction(first);

        }


}

void Mcg::printCode()
{
    // print prologue
    printf("\t.section .rodata\n"
           "s_readint_fmt: .string \"%%ld\"\n"

           "s_writeint_fmt: .string \"%%ld\\n\"\n"
           "\t.section .bss\n"
           "\t.align 8\n"

           "s_readbuf: .space 8\n"
           "\t.section .text\n"

           "\t.globl main\n"
           "main:\n");
    

    int space = myTable->vrIdxMax * 8 + myTable->structSize + 16; // vr size + array&record size
    if (space % 16 == 0)
        space += 8;
    printf("\tsubq $%d, %%rsp\n", space);

    // use printer ...
    PrintX86_64InstructionSequence print_code(outSeq);
    print_code.print();

    // print epilogue
    printf("\taddq $%d, %%rsp\n", space);
    printf("\tmovl $0, %%eax\n"
           "\tret\n");
}

void Mcg::updateLiveTime(BasicBlock* curBlock){
    std::vector<Instruction *>::iterator it = curBlock->begin();
    int begin;
    int cnt = 1;

    while(it != curBlock->end()){
        // do not count the jmp's operand
        if((*it)->get_opcode() == HINS_JUMP ||          // no operand or operand is not vr
            (*it)->get_opcode() == HINS_JE||
            (*it)->get_opcode() == HINS_JNE||
            (*it)->get_opcode() == HINS_JLT|| 
            (*it)->get_opcode() == HINS_JLTE|| 
            (*it)->get_opcode() == HINS_JGT|| 
            (*it)->get_opcode() == HINS_JGTE|| 
            (*it)->get_opcode() == HINS_NOP
            ){
                it++;
                continue;
            }

        if(                                             //(*it)->get_opcode() == HINS_LOAD_INT ||
            (*it)->get_opcode() == HINS_STORE_INT       // all vr are use
            ||(*it)->get_opcode() == HINS_INT_COMPARE
            ||(*it)->get_opcode() == HINS_WRITE_INT){
            begin = 0;
        }else{                                          // the first vr is store, others are used
            begin = 1;
            Operand* vr = new Operand();
            *vr = (*it)->get_operand(0);
            if(vr->get_kind() == OPERAND_VREG && liveTime.count(vr->get_base_reg())==0 ){     //if one vr already has a value, do not need -1 again
                liveTime[vr->get_base_reg()] = -1;      //inorder to let all vr have one value
            }
        } 

        for(unsigned int i=begin;i<(*it)->get_num_operands();i++){
            Operand* vr = new Operand();
            *vr = (*it)->get_operand(i);
            if(vr->get_kind() == OPERAND_VREG){
                liveTime[vr->get_base_reg()] = cnt;
            }
        }

        it++;
        cnt++;
    }


}


Operand* Mcg::getMrUse(int vrIdx){   //return a mr that contains value to use;
    // vr already in registers
    if(vrToMr.count(vrIdx) !=0 ){
        int mrIdx = vrToMr[vrIdx];
        return mr[mrIdx];
    }else if(spill.count(vrIdx)){          //vr that is spilled
        Operand* spareMr = getMrStore(vrIdx);


        int sOffset = vrIdx* 8 + myTable->structSize;
        Operand src(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, sOffset);  //restore location

        spill.erase(vrIdx);  // this vr is restored

        Instruction *first = new Instruction(MINS_MOVQ, src, *spareMr);  //restore instruction
        first->set_comment("restore");
        curBlock->add_instruction(first);

        return spareMr;
    }else{
        cerr<<"Error:vr"<<vrIdx<<" is not assigned before"<<endl;
        exit(-1);
    }


}



Operand* Mcg::getMrStore(int vrIdx){   //return a mr that can be used to store;

    // this vr already has been assigned a mr 
    if(vrToMr.count(vrIdx)){
        return mr[vrToMr[vrIdx]];
    }

    // vr has not been assigned a mr

    int spare = -1;
    for(int i=0;i<mrToVr.size();i++){
        if(mrToVr[i] == -1){
            spare = i;
            break;
        }
    }
    //assign a new mr;
    if(spare != -1){
        //have spare mr, assgin a mr to vr

        Operand* curMr = mr[spare];
        // repeated vr ask for stroage
        if(vrToMr.count(vrIdx) != 0){
            int mrIdx = vrToMr[vrIdx];
            vrToMr.erase(vrIdx);
            mrToVr[mrIdx]=-1;
        }

        vrToMr[vrIdx] = spare;
        mrToVr[spare] = vrIdx;
        return curMr;

    }else{
        // how set how many memory to store spilled value?
        // has dead vr
        std::unordered_map<int,int>::iterator it = liveTime.begin();
        while(it != liveTime.end()){
            // if the vr is using an mr , its latest use ins is smaller than current ins and it is not a globle variable 
            if( vrToMr.count(it->first) != 0 && it->second < insIdx && it->first>=myTable->vrIdx){
                int mrIdx = vrToMr[it->first]; 


                vrToMr.erase(it->first);
                mrToVr[mrIdx]=-1;
                vrToMr[vrIdx] = mrIdx;
                mrToVr[mrIdx] = vrIdx;
                return mr[mrIdx];
            }
            it++;
        }

        //spill
        int lastUseVr = -1;          // find the vr that will be used in farest time
        int tmpMax = -1;  // tmp value to record max idx of statement 
        it = liveTime.begin();


        while(it != liveTime.end()){
            // currently used , not spilled
            if( vrToMr.count(it->first) != 0 && spill.count(it->first) == 0 && it->second > tmpMax){

                tmpMax = it->second;
                lastUseVr = it->first;
            }
            it++;
        }

        Operand* curMr = mr[vrToMr[lastUseVr]]; // mr to return;

        Operand src = *(curMr);      //value of vr to spill

        int dOffset = lastUseVr * 8 + myTable->structSize;
        Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);  //spill location


        Instruction *first = new Instruction(MINS_MOVQ, src, des);  //spill instruction
        string cmt = "spill vr" + to_string(lastUseVr);
        first->set_comment(cmt);
        curBlock->add_instruction(first);

        spill[lastUseVr] = dOffset;  // record this vr is spilled

        // add new record;
        int mrIdx = vrToMr[lastUseVr];
        // delete original record
        vrToMr.erase(lastUseVr);
        mrToVr[mrIdx]=-1;

        vrToMr[vrIdx] =  mrIdx;     //update maps
        mrToVr[mrIdx] = vrIdx;

        return curMr;

    }
    return nullptr;

}

void Mcg::storeCallerMr(){   // store all the caller saved mr to memory
    int vrIdx,dOffset;
    Operand src;
     for(int i=5;i<9;i++){
        if(mrToVr[i] != -1){
            vrIdx = mrToVr[i];
            src = *(mr[i]);      //value of vr to spill

            dOffset = vrIdx* 8 + myTable->structSize;
            Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);  //store location


            Instruction *first = new Instruction(MINS_MOVQ, src, des);  //store instruction
            first->set_comment("store Caller Mr");
            curBlock->add_instruction(first);
        }
    }
}

void Mcg::loadCallerMr(){   // load all the caller saved mr to memory
    int vrIdx,dOffset;
    Operand src;
     for(int i=5;i<9;i++){
        if(mrToVr[i] != -1){
            vrIdx = mrToVr[i];
            src = *(mr[i]);      //value of vr to spill

            dOffset = vrIdx* 8 + myTable->structSize;
            Operand des(OPERAND_MREG_MEMREF_OFFSET, MREG_RSP, dOffset);  //load location


            Instruction *first = new Instruction(MINS_MOVQ, des, src);  //load instruction
            first->set_comment("load Caller Mr");
            curBlock->add_instruction(first);
        }
    }
}


