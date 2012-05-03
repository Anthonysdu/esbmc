/*******************************************************************\

Module: Slicer for symex traces

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <hash_cont.h>

#include "slice.h"

/*******************************************************************\

   Class: symex_slicet

 Purpose:

\*******************************************************************/

class symex_slicet
{
public:
  void slice(symex_target_equationt &equation);

protected:
  typedef hash_set_cont<irep_idt, irep_id_hash> symbol_sett;
  
  symbol_sett depends;
  
  void get_symbols(const expr2tc &expr);
  void get_symbols(const type2tc &type);

  void slice(symex_target_equationt::SSA_stept &SSA_step);
  void slice_assignment(symex_target_equationt::SSA_stept &SSA_step);
};

/*******************************************************************\

Function: symex_slicet::get_symbols

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void symex_slicet::get_symbols(const expr2tc &expr)
{
  get_symbols(expr->type);

  std::vector<const expr2tc*> operands;
  expr->list_operands(operands);
  
  for (std::vector<const expr2tc*>::const_iterator it = operands.begin();
       it != operands.end(); it++)
    get_symbols(**it);

  if (is_symbol2t(expr))
    depends.insert(symbol2tc(expr)->name);
}

/*******************************************************************\

Function: symex_slicet::get_symbols

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void symex_slicet::get_symbols(const type2tc &type)
{
}

/*******************************************************************\

Function: symex_slicet::slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void symex_slicet::slice(symex_target_equationt &equation)
{
  depends.clear();

  for(symex_target_equationt::SSA_stepst::reverse_iterator
      it=equation.SSA_steps.rbegin();
      it!=equation.SSA_steps.rend();
      it++)
    slice(*it);
}

/*******************************************************************\

Function: symex_slicet::slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void symex_slicet::slice(symex_target_equationt::SSA_stept &SSA_step)
{
  get_symbols(SSA_step.guard);

  switch(SSA_step.type)
  {
  case goto_trace_stept::ASSERT:
    get_symbols(SSA_step.cond);
    break;

  case goto_trace_stept::ASSUME:
    get_symbols(SSA_step.cond);
    break;

  case goto_trace_stept::ASSIGNMENT:
    slice_assignment(SSA_step);
    break;

  case goto_trace_stept::OUTPUT:
    break;

  default:
    assert(false);  
  }
}

/*******************************************************************\

Function: symex_slicet::slice_assignment

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void symex_slicet::slice_assignment(
  symex_target_equationt::SSA_stept &SSA_step)
{
  assert(is_symbol2t(SSA_step.lhs));

  if(depends.find(symbol2tc(SSA_step.lhs)->name) == depends.end())
  {
    // we don't really need it
    SSA_step.ignore=true;
  }
  else
    get_symbols(SSA_step.rhs);
}

/*******************************************************************\

Function: slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void slice(symex_target_equationt &equation)
{
  symex_slicet symex_slice;
  symex_slice.slice(equation);
}

/*******************************************************************\

Function: simple_slice

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void simple_slice(symex_target_equationt &equation)
{
  // just find the last assertion
  symex_target_equationt::SSA_stepst::iterator
    last_assertion=equation.SSA_steps.end();
  
  for(symex_target_equationt::SSA_stepst::iterator
      it=equation.SSA_steps.begin();
      it!=equation.SSA_steps.end();
      it++)
    if(it->is_assert())
      last_assertion=it;

  // slice away anything after it

  symex_target_equationt::SSA_stepst::iterator s_it=
    last_assertion;

  if(s_it!=equation.SSA_steps.end())
    for(s_it++;
        s_it!=equation.SSA_steps.end();
        s_it++)
      s_it->ignore=true;
}

