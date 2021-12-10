#include <cassert>
#include <vector>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include "node.h"
#include "grammar_symbols.h"
#include "ast.h"
#include "type.h"
#include "symbol.h"
#include "symtab.h"
#include "cfg.h"
#include "highlevel.h"
#include "optimizer.h"
#include "live_vregs.h"
#include <bitset>
using namespace std;


VNKey::VNKey(int input1,int input2,int input3){
        opcode = input1;
        vn1 = input2;
        vn2 = input3;
}
////////////////////////////////////////////////////////////////////////
// Optimizer Implementation
////////////////////////////////////////////////////////////////////////

Optimizer::Optimizer(){
  src = nullptr;
  dst = new ControlFlowGraph();
  curBlock = nullptr;


}

BasicBlock* Optimizer::findBlock(ControlFlowGraph* cfg,unsigned int bbIdx){
  for (auto i =cfg->bb_begin(); i != cfg->bb_end(); i++) {
      if( (*i)->get_id() == bbIdx) return (*i);
  }
  cerr<<"Error: Cannot find the block with idx: "<<bbIdx<<endl;
  exit(-1);
}

void Optimizer::optimize(){
    visitCfg(src,dst);
}

void Optimizer::resetCfg(ControlFlowGraph* input){
  src = input;
  dst = new ControlFlowGraph();
  curBlock = nullptr;
}

void Optimizer::visitCfg(ControlFlowGraph* input,ControlFlowGraph* output){

  vector<Edge*> myEdgeList;
    //deal with block
  for (auto i = input->bb_begin(); i != input->bb_end(); i++) {
    BasicBlock *bb = *i;
    BasicBlockKind bb_kind = bb->get_kind();


     if (bb->has_label()) {
         curBlock= output->create_basic_block(bb->get_kind(),bb->get_label());
     }else{
         curBlock= output->create_basic_block(bb->get_kind(),"");
     }

    if (bb_kind == BASICBLOCK_INTERIOR) {
        this->visitBlock(bb);
    }
  }

  //deal with edge
  BasicBlock* target;
  BasicBlock* newSource;
  BasicBlock* newTarget;

  for (auto i = input->bb_begin(); i != input->bb_end(); i++) {
    BasicBlock *bb = *i;
    myEdgeList = input->get_outgoing_edges(bb);

    for(int j=0;j<myEdgeList.size();j++){
        target = myEdgeList[j]->get_target();
        newSource = findBlock(output,bb->get_id());
        newTarget= findBlock(output,target->get_id());
        output->create_edge(newSource,newTarget,myEdgeList[j]->get_kind());
    }
  }

}

void Optimizer::visitBlock(BasicBlock* inputBlock){
    cout<<"Please overwrite it"<<endl;;
}

void Optimizer::visitIns(Instruction* curIns){
    cout<<"Please overwrite it"<<endl;
}




////////////////////////////////////////////////////////////////////////
// Local Value Numbering implementation
////////////////////////////////////////////////////////////////////////

ValueNumbering::ValueNumbering(ControlFlowGraph* input){
    src = input;
    dst = new ControlFlowGraph();
    curBlock = nullptr;
    nextVN = 0;
    memOffset = 10000;
}

void ValueNumbering::visitBlock(BasicBlock* curBlock){
    nextVN = 0;
    constToVn.clear();
    vnToConst.clear(); 
    vrToVn.clear(); 
    vnToVr.clear();
    keyToVn.clear();

    // begin going through
    int sz = curBlock->get_length();
    Instruction *ins;
    string label;

    for (int i = 0; i < sz; i++)
    {
        ins = curBlock->get_instruction(i);
        visitIns(ins);
    }
}

void ValueNumbering::handleRead(Instruction* curIns){    
  int curVr = curIns->get_operand(0).get_base_reg();
  int oldVn = -1;
  if(vrToVn.count(curVr)){
    oldVn = vrToVn[curVr];
    vrToVn.erase(curVr);
    if(vnToVr[oldVn] == curVr) vnToVr.erase(oldVn);
  }
  vrToVn[curVr] = nextVN;
  vnToVr[nextVN] = curVr;
  nextVN++;

  curBlock->add_instruction(curIns);
}

void ValueNumbering::handleMemory(Instruction* curIns){    
  // HINS_LOCALADDR        
  int curVr = curIns->get_operand(0).get_base_reg();
  int oldVn = -1;
  if(vrToVn.count(curVr)){
    oldVn = vrToVn[curVr];
    vrToVn.erase(curVr);
    if(vnToVr[oldVn] == curVr) vnToVr.erase(oldVn);
  }
  int newVn;
  int memAdd = curIns->get_operand(1).get_int_value() + memOffset;

  if(constToVn.count(memAdd)){
    newVn = constToVn[memAdd];
    vrToVn[curVr] = newVn;
    vnToVr[newVn] = curVr;
  }else{
    constToVn[memAdd] = nextVN;
    vnToConst[nextVN] = memAdd;

    vrToVn[curVr] = nextVN;
    vnToVr[nextVN] = curVr;
    nextVN++;
  }

  curBlock->add_instruction(curIns);

}

void ValueNumbering::handleMov(Instruction* curIns){    

    Operand tmp;
    int curVn;
    tmp = curIns->get_operand(1); 
    int srcVr;
    if(tmp.get_kind() == OPERAND_INT_LITERAL){
      srcVr= tmp.get_int_value();
    }else{
      srcVr= tmp.get_base_reg();
    }

    bool isInVntoVr = 0;


    // if the source vr is already in map, take its vn as curVn, otherwise create a new vn as current vn

    if(tmp.get_kind() == OPERAND_INT_LITERAL){
      // if the case is vr1 = 3; remove the original record of vr1 and assign new vn for vr1;

      if(constToVn.count(srcVr)){
         curVn = constToVn[srcVr];
       }else{
        curVn = nextVN;
        constToVn[srcVr] = curVn;
        vnToConst[curVn] = srcVr;
        nextVN++;
      }
    }else if(tmp.get_kind() == OPERAND_VREG){
      if(vrToVn.count(srcVr)){
        curVn = vrToVn[srcVr];
        isInVntoVr = 1; 
      }else{
        curVn = nextVN;
        vrToVn[srcVr] = curVn;
        vnToVr[curVn] = srcVr;
        nextVN++;
      }
    }else{
      curBlock->add_instruction(curIns);
      return;
    }

    if(curIns->get_operand(0).get_kind() != OPERAND_VREG){
      curBlock->add_instruction(curIns);
      return;
    }

    int dstVr = curIns->get_operand(0).get_base_reg();

    //if the dstVr is in the map, remove original records and add new records into the maps
      if(vrToVn.count(dstVr)){
        int tmpVn = vrToVn[dstVr];
        vrToVn.erase(dstVr);
        if(vnToVr[tmpVn] == dstVr) vnToVr.erase(tmpVn);
      }

      vrToVn[dstVr] = curVn;
      if(!isInVntoVr) vnToVr[curVn] = dstVr; 

      curBlock->add_instruction(curIns);
}

void ValueNumbering::handleExp(Instruction* curIns){

  int operandVr1, operandVr2, resVr;
  int operandVn1, operandVn2, resVn;

  resVn = -1;

  resVr = curIns->get_operand(0).get_base_reg();



  int symbol;
  symbol= curIns->get_opcode();

  if(curIns->get_operand(1).get_kind() == OPERAND_VREG){
    operandVr1 = curIns->get_operand(1).get_base_reg();
    if(vrToVn.count(operandVr1)){
      operandVn1 = vrToVn[operandVr1];
    }else{
      operandVn1 = nextVN;
      vrToVn[operandVr1] = nextVN;
      vnToVr[nextVN] = operandVn1;
      nextVN++;
    }
  }else if(curIns->get_operand(1).get_kind() == OPERAND_INT_LITERAL){
    operandVr1 = curIns->get_operand(1).get_int_value();
    if(constToVn.count(operandVr1)){
      operandVn1 = constToVn[operandVr1]; 
    }else{
      operandVn1 = nextVN;
      constToVn[operandVr1] = nextVN;
      vnToConst[nextVN] = operandVn1;
      nextVN++;
    }
  }

  if(curIns->get_operand(2).get_kind() == OPERAND_VREG){
    operandVr2 = curIns->get_operand(2).get_base_reg();
    if(vrToVn.count(operandVr2)){
      operandVn2 = vrToVn[operandVr2];
    }else{
      operandVn2 = nextVN;
      vrToVn[operandVr2] = nextVN;
      vnToVr[nextVN] = operandVn2;
      nextVN++;
    }
  }else if(curIns->get_operand(2).get_kind() == OPERAND_INT_LITERAL){
    operandVr2 = curIns->get_operand(2).get_int_value();
    if(constToVn.count(operandVr2)){
      operandVn2 = constToVn[operandVr2]; 
    }else{
      operandVn2 = nextVN;
      constToVn[operandVr2] = nextVN;
      vnToConst[nextVN] = operandVn2;
      nextVN++;
    }
  }


  // if the calculation is existed
  unordered_map<VNKey*,int>::iterator it = keyToVn.begin();

  while(it != keyToVn.end()){
    if(it->first->opcode == symbol && it->first->vn1 == operandVn1 && it->first->vn2 == operandVn2)
    {
      resVn = it->second;
      break;
    }
    it++;
  }

  // if the calculation is not existed
  if(resVn == -1){
    //update keyToVn map
    VNKey* tmpKey = new VNKey(symbol,operandVn1,operandVn2);
    keyToVn[tmpKey] = nextVN;
    //update vrToVn and vnToVr map
    vrToVn[resVr] = nextVN;
    vnToVr[nextVN] = resVr;
    nextVN++;

    curBlock->add_instruction(curIns);
    return;
  }

  //add dst into vrToVn 
  vrToVn[resVr] = resVn;

  int sourceVr;
  if(vnToVr.count(resVn)){
    sourceVr= vnToVr[resVn];
  }else{
    curBlock->add_instruction(curIns);
    return;
  }
   
  Operand srcOperand(OPERAND_VREG,sourceVr);

  Instruction* ins = new Instruction(HINS_MOV,curIns->get_operand(0),srcOperand);
  ins->set_comment("LVN: replace");
  curBlock->add_instruction(ins);

}

void ValueNumbering::visitIns(Instruction* curIns){
  int opcode = curIns->get_opcode();
  switch (opcode) {
    //expression
  case HINS_INT_ADD:     
  case HINS_INT_SUB:     
  case HINS_INT_MUL:    
  case HINS_INT_DIV:    
  case HINS_INT_MOD:    
  case HINS_INT_NEGATE: 
      handleExp(curIns);
      break;
  //do nothing
  case HINS_NOP:         
  case HINS_JUMP:        
  case HINS_JE:         
  case HINS_JNE:       
  case HINS_JLT:      
  case HINS_JLTE:        
  case HINS_JGT:        
  case HINS_JGTE:       
  case HINS_INT_COMPARE:
  case HINS_WRITE_INT:  
  case HINS_STORE_INT:  
      curBlock->add_instruction(curIns);
      break;
  //mov
  case HINS_LOAD_ICONST: 
  case HINS_MOV:         
      handleMov(curIns);
      break;
  //memory 
  case HINS_LOCALADDR: 
      handleMemory(curIns);
      break;
  //read
  case HINS_LOAD_INT:  
  case HINS_LEA:        
  case HINS_READ_INT:    
      handleRead(curIns);
      break;

  default:
    assert(false);
  }

}

////////////////////////////////////////////////////////////////////////
// ConstProp Implementation
////////////////////////////////////////////////////////////////////////
ConstProp::ConstProp(ControlFlowGraph* input){
    src = input;
    dst = new ControlFlowGraph();
    curBlock = nullptr;
}

void ConstProp::visitBlock(BasicBlock* curBlock){
    vrToConst.clear();

    // begin going through
    int sz = curBlock->get_length();
    Instruction *ins;

    for (int i = 0; i < sz; i++)
    {
        ins = curBlock->get_instruction(i);
        visitIns(ins);
    }
}

void ConstProp::visitIns(Instruction* curIns){

  if(curIns->get_opcode() == HINS_LOAD_ICONST){
    int num = curIns->get_operand(1).get_int_value();
    int vrIdx = curIns->get_operand(0).get_base_reg();

    vrToConst[vrIdx] = num;
    curBlock->add_instruction(curIns);

  }else if(curIns->get_num_operands() == 2 || curIns->get_num_operands() == 3 ){

    vector<Operand> newOperand;

    for(int i=0;i<curIns->get_num_operands();i++){
      if(curIns->get_operand(i).get_kind() == OPERAND_VREG && 
         vrToConst.count(curIns->get_operand(i).get_base_reg())){
           Operand tmp(OPERAND_INT_LITERAL, vrToConst[curIns->get_operand(i).get_base_reg()]);
           newOperand.push_back(tmp);
      }else{
        newOperand.push_back(curIns->get_operand(i));
      }
    }
    Instruction* ins;
    if(curIns->get_num_operands() == 2){
     ins = new Instruction(curIns->get_opcode(),newOperand[0],newOperand[1]);
    }else{
     ins = new Instruction(curIns->get_opcode(),newOperand[0],newOperand[1],newOperand[2]);
    }

    curBlock->add_instruction(ins);

  }else{
    curBlock->add_instruction(curIns);
  }
}

////////////////////////////////////////////////////////////////////////
// VRProp Implementation
////////////////////////////////////////////////////////////////////////
 VRProp::VRProp(ControlFlowGraph* input){
    src = input;
    dst = new ControlFlowGraph();
    curBlock = nullptr;
}

void VRProp::visitBlock(BasicBlock* curBlock){
    vrToVr.clear();

    // begin going through
    int sz = curBlock->get_length();
    Instruction *ins;

    for (int i = 0; i < sz; i++)
    {
        ins = curBlock->get_instruction(i);
        visitIns(ins);
    }
}

void VRProp::visitIns(Instruction* curIns){

  if(curIns->get_opcode() == HINS_MOV && curIns->get_operand(1).get_kind() == OPERAND_VREG){
    int vr0 = curIns->get_operand(0).get_base_reg();
    int vr1 = curIns->get_operand(1).get_base_reg();

    if(vrToVr.count(vr1)){
      vrToVr[vr0] = vrToVr[vr1];
    }else{
      vrToVr[vr0] = vr1;
    }
  }
  
  if(curIns->get_num_operands() == 2 || curIns->get_num_operands() == 3 ){

    vector<Operand> newOperand;
    Operand curOp;

    for(int i=0;i<curIns->get_num_operands();i++){
      curOp = curIns->get_operand(i);
      if(is_use(curIns,i) && vrToVr.count(curOp.get_base_reg())){
           Operand tmp(OPERAND_VREG, vrToVr[curOp.get_base_reg()]);
           newOperand.push_back(tmp);
      }else{
          newOperand.push_back(curOp);
      }
    }
    Instruction* ins;
    if(curIns->get_num_operands() == 2){
     ins = new Instruction(curIns->get_opcode(),newOperand[0],newOperand[1]);

    }else{
     ins = new Instruction(curIns->get_opcode(),newOperand[0],newOperand[1],newOperand[2]);
    }

    curBlock->add_instruction(ins);

  }else{
    curBlock->add_instruction(curIns);
  }
}

////////////////////////////////////////////////////////////////////////
// Peephole Implementation
////////////////////////////////////////////////////////////////////////
Peephole::Peephole(ControlFlowGraph* input){
    src = input;
    dst = new ControlFlowGraph();
    curBlock = nullptr;
}
void Peephole::replace(Instruction* ins){
    bool shouldReplace = 0;
    if(vrToVr.size()){
      vector<Operand> operands;
      for(int i=0;i<ins->get_num_operands();i++){
        if(ins->get_operand(i).has_base_reg() && vrToVr.count(ins->get_operand(i).get_base_reg())){
          Operand newOperand(OPERAND_VREG,vrToVr[ins->get_operand(i).get_base_reg()]);
          operands.push_back(newOperand);
          shouldReplace = 1; 

        }else{
          operands.push_back(ins->get_operand(i));
        }
      }
    
    Instruction* newIns;
    if(shouldReplace){
      if(ins->get_num_operands() == 2){
        newIns = new Instruction(ins->get_opcode(),operands[0],operands[1]);
      }else if (ins->get_num_operands() == 3){
        newIns = new Instruction(ins->get_opcode(),operands[0],operands[1],operands[2]);
      }else{
        cerr<<"Error: The Peephole replace cannot handle this case"<<endl;
        exit(-1);
      }
      curBlock->add_instruction(newIns);

    }else{
      curBlock->add_instruction(ins);
    }
  }else{
      curBlock->add_instruction(ins);
  }
}
bool Peephole::ldiMovRule(BasicBlock* inputBlock, int i){
    Instruction *ins0 = inputBlock->get_instruction(i);
    if(i+1 <inputBlock->get_length()){
      Instruction *ins1 = inputBlock->get_instruction(i+1);
      if(ins0->get_opcode() == HINS_LOAD_INT 
        && ins1->get_opcode() == HINS_MOV
        && ins0->get_operand(0).has_base_reg() 
        && ins1->get_operand(1).has_base_reg() 
        && ins0->get_operand(0).get_base_reg() == ins1->get_operand(1).get_base_reg()){
          vrToVr[ins1->get_operand(1).get_base_reg()] = ins1->get_operand(0).get_base_reg();

          Instruction* ins = new Instruction(HINS_LOAD_INT, ins1->get_operand(0),ins0->get_operand(1));
          ins->set_comment("Peephole: ldiMov");
          curBlock->add_instruction(ins);
          return true;
      }

    }

    return false;

}
void Peephole::visitBlock(BasicBlock* inputBlock){
    // begin going through
    int sz = inputBlock->get_length();

    Instruction *curIns;
    for (int i = 0; i < sz; i++)
    {
        curIns = inputBlock->get_instruction(i);

        if(ldiMovRule(inputBlock,i)){
          i++;
          continue;
        }

        replace(curIns);

    }
}
void Peephole::visitIns(Instruction* curIns){

}

////////////////////////////////////////////////////////////////////////
// Cleaner Implementation
////////////////////////////////////////////////////////////////////////
Cleaner::Cleaner(ControlFlowGraph* input){
    src = input;
    dst = new ControlFlowGraph();
    curBlock = nullptr;
    analyzer = new LiveVregs(input);
    analyzer->execute();
}
void Cleaner::visitBlock(BasicBlock* inputBlock){
    // begin going through
    int sz = inputBlock->get_length();
    Instruction *ins;
    bitset<256> liveVr;

    for (int i = 0; i < sz; i++)
    {

        ins = inputBlock->get_instruction(i);
        liveVr = analyzer->get_fact_after_instruction(inputBlock,ins);
        if(is_def(ins)){
          int vr = ins->get_operand(0).get_base_reg();
          if(liveVr.test(vr)){
            curBlock->add_instruction(ins);
          }

        }else{
            curBlock->add_instruction(ins);
        }
    }
}
void Cleaner::visitIns(Instruction* curIns){

}


