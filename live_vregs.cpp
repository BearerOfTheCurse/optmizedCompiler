#include <algorithm>
#include <iostream>
#include <string>
#include "cfg.h"
#include "highlevel.h"
#include "live_vregs.h"
using namespace std;

LiveVregs::LiveVregs(ControlFlowGraph *cfg)
  : m_cfg(cfg)
  , m_endfacts(cfg->get_num_blocks(), LiveSet())
  , m_beginfacts(cfg->get_num_blocks(), LiveSet()) {
}

LiveVregs::~LiveVregs() {
}

void LiveVregs::execute() {
  compute_iter_order();

/*
  // for now just print iteration order
  printf("Iteration order:\n");
  for (auto i = m_iter_order.begin(); i != m_iter_order.end(); i++) {
    printf("%u ", *i);
  }
  printf("\n");
*/

  bool done = false;

  unsigned num_iters = 0;
  while (!done) {
    num_iters++;
    bool change = false;

    // for each block (according to the iteration order)...
    for (auto i = m_iter_order.begin(); i != m_iter_order.end(); i++) {
      unsigned id = *i;
      BasicBlock *bb = m_cfg->get_block(id);

      // Compute the set of vregs we currently know to be alive at the
      // end of the basic block.  For the exit block, this is the empty set.
      // for all other blocks (which will have at least one successor),
      // then it's the union of the vregs we know to be alive at the
      // beginning of each successor.
      LiveSet live_set;
      if (bb->get_kind() != BASICBLOCK_EXIT) {
        const ControlFlowGraph::EdgeList &outgoing_edges = m_cfg->get_outgoing_edges(bb);
        for (auto j = outgoing_edges.cbegin(); j != outgoing_edges.cend(); j++) {
          Edge *e = *j;
          BasicBlock *successor = e->get_target();
          live_set |= m_beginfacts[successor->get_id()];
        }
      }

      // Update (currently-known) live vregs at the end of the basic block
      m_endfacts[id] = live_set;

      // for each Instruction in reverse order...
      for (auto j = bb->crbegin(); j != bb->crend(); j++) {
        Instruction *ins = *j;

        // model the instruction
        model_instruction(ins, live_set);
      }

      // did the live_set at the beginning of the block change?
      // if so, at least one more round will be needed
      if (live_set != m_beginfacts[id]) {
        change = true;
        m_beginfacts[id] = live_set;
      }
    }

    // if there was no change in the computed dataflow fact at the beginning
    // of any block, then the analysis has terminated
    if (!change) {
      done = true;
    }
  }
  //printf("Analysis finished in %u iterations\n", num_iters);
}

const LiveVregs::LiveSet &LiveVregs::get_fact_at_end_of_block(BasicBlock *bb) const {
  return m_endfacts.at(bb->get_id());
}

const LiveVregs::LiveSet &LiveVregs::get_fact_at_beginning_of_block(BasicBlock *bb) const {
  return m_beginfacts.at(bb->get_id());
}

LiveVregs::LiveSet LiveVregs::get_fact_after_instruction(BasicBlock *bb, Instruction *ins) const {
  LiveSet live_set = m_endfacts[bb->get_id()];

  for (auto i = bb->crbegin(); i != bb->crend(); i++) {
    if (*i == ins) {
      break;
    }
    model_instruction(*i, live_set);
  }

  return live_set;
}

LiveVregs::LiveSet LiveVregs::get_fact_before_instruction(BasicBlock *bb, Instruction *ins) const {
  LiveSet live_set = m_endfacts[bb->get_id()];

  for (auto i = bb->crbegin(); i != bb->crend(); i++) {
    model_instruction(*i, live_set);
    if (*i == ins) {
      break;
    }
  }

  return live_set;
}

void LiveVregs::compute_iter_order() {
  // since this is a backwards problem,
  // desired iteration order is reverse postorder on
  // reversed CFG
  std::bitset<MAX_BLOCKS> visited;
  postorder_on_rcfg(visited, m_cfg->get_exit_block());
  std::reverse(m_iter_order.begin(), m_iter_order.end());
}

void LiveVregs::postorder_on_rcfg(std::bitset<LiveVregs::MAX_BLOCKS> &visited, BasicBlock *bb) {
  // already arrived at this block?
  if (visited.test(bb->get_id())) {
    return;
  }

  // this block is now guaranteed to be visited
  visited.set(bb->get_id());

  // recursively visit predecessors
  const ControlFlowGraph::EdgeList &incoming_edges = m_cfg->get_incoming_edges(bb);
  for (auto i = incoming_edges.begin(); i != incoming_edges.end(); i++) {
    Edge *e = *i;
    postorder_on_rcfg(visited, e->get_source());
  }

  // add this block to the order
  m_iter_order.push_back(bb->get_id());
}

string intToHighType(int input){
  switch (input) {
  case HINS_NOP:         return "nop";
  case HINS_LOAD_ICONST: return "ldci";
  case HINS_INT_ADD:     return "addi";
  case HINS_INT_SUB:     return "subi";
  case HINS_INT_MUL:     return "muli";
  case HINS_INT_DIV:     return "divi";
  case HINS_INT_MOD:     return "modi";
  case HINS_INT_NEGATE:  return "negi";
  case HINS_LOCALADDR:   return "localaddr";
  case HINS_LOAD_INT:    return "ldi";
  case HINS_STORE_INT:   return "sti";
  case HINS_READ_INT:    return "readi";
  case HINS_WRITE_INT:   return "writei";
  case HINS_JUMP:        return "jmp";
  case HINS_JE:          return "je";
  case HINS_JNE:         return "jne";
  case HINS_JLT:         return "jlt";
  case HINS_JLTE:        return "jlte";
  case HINS_JGT:         return "jgt";
  case HINS_JGTE:        return "jgte";
  case HINS_INT_COMPARE: return "cmpi";
  case HINS_MOV:         return "mov";
  case HINS_LEA:         return "lea";

  default:
    assert(false);
    return "<invalid>";
  }
}
void LiveVregs::model_instruction(Instruction *ins, LiveSet &fact) const {
  // Model an instruction (backwards).  If the instruction is a def,
  // it kills any vreg that was live.  Every use in the instruction
  // creates a live vreg (or keeps the vreg alive).

  // if (HighLevel::is_def(ins)) {
  //cout<<"Debug: "<< intToHighType(ins->get_opcode()) <<endl;

  if (is_def(ins)) {
    Operand operand = ins->get_operand(0);
    assert(operand.has_base_reg());
    fact.reset(operand.get_base_reg());
  }

  for (unsigned i = 0; i < ins->get_num_operands(); i++) {
    // if (HighLevel::is_use(ins, i)) {
    if (is_use(ins, i)) {
      Operand operand = ins->get_operand(i); 

      assert(operand.has_base_reg());
      fact.set(operand.get_base_reg());

      if (operand.has_index_reg()) {
        fact.set(operand.get_index_reg());
      }
    }
  }
}