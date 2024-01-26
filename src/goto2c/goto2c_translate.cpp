#include <goto2c/goto2c.h>
#include <goto2c/expr2c.h>
#include <util/expr_util.h>
#include <util/c_sizeof.h>
#include <util/config.h>

// This translates the given GOTO program (aka list of GOTO functions)
std::string goto2ct::translate()
{
  std::ostringstream out;

  // Banner
  out << "///////////////////////////////////////////////////////\n";
  out << "//\n";
  out << "// This program is generated by the GOTO2C translator\n";
  out << "//\n";
  out << "///////////////////////////////////////////////////////\n";
  out << "\n\n\n";

  // Includes
  out << "///////////////////   HEADERS   ///////////////////////\n";
  out << "#include <esbmc_intrinsics.h> // included automatically\n";
  out << "\n\n";

  // Types
  out << "/////////////////////   GLOBAL TYPES  ////////////////////////\n";
  for (auto type : global_types)
    out << typedef2c(type, ns) << "; // " << type.location() << "\n";
  out << "\n\n";

  // External variables
  out << "/////////////////  EXTERNAL VARIABLES   ////////////////////\n";
  for (auto s : extern_vars)
    out << expr2c(code_declt(symbol_expr(s)), ns) << "; // " << s.location
        << "\n";
  out << "\n\n";

  // Function declarations
  out << "/////////////////  FUNCTION DECLARATIONS   ////////////////////\n";
  for (auto s : fun_decls)
    out << expr2c(code_declt(symbol_expr(s)), ns) << "; // " << s.location
        << "\n";
  out << "\n\n";

  // Global variables declarations
  out << "//////////////// GLOBAL VARIABLE DECLARATIONS ///////////////////\n";
  for (auto s : global_vars)
    out << expr2c(code_declt(symbol_expr(s)), ns) << "; // " << s.location
        << "\n";
  out << "\n\n";

  // Global variables definitions
  out << "//////////////// GLOBAL VARIABLE DEFINITIONS ///////////////////\n";
  for (auto s : global_vars)
  {
    if (initializers.count(s.id.as_string()) > 0)
    {
      out << expr2c(
               code_declt(symbol_expr(s), initializers[s.id.as_string()]), ns)
          << "; // " << s.location << "\n";
    }
  }
  out << "\n\n";

  // Function definitions
  out << "/////////////////   FUNCTION DEFINITIONS   ////////////////////\n";
  out << translate(goto_functions) << "\n";

  out << "////////////////////// PROGRAM END /////////////////////////////\n";
  return out.str();
}

// Convert GOTO functions (without the context) to C
std::string goto2ct::translate(goto_functionst &goto_functions)
{
  std::ostringstream out;

  // Iterating through the available GOTO functions
  for (auto &it : goto_functions.function_map)
    out << translate(it.first.as_string(), it.second) << "\n";

  return out.str();
}

// Convert GOTO function (without the context) to C
std::string
goto2ct::translate(std::string function_id, goto_functiont &goto_function)
{
  std::ostringstream out;

  std::string fun_name_short = expr2ct::get_name_shorthand(function_id);
  // Creating a function symbol
  symbolt fun_sym;
  fun_sym.id = function_id;
  fun_sym.type = goto_function.type;
  // Translating the function declaration
  out << expr2c(code_declt(symbol_expr(fun_sym)), ns) << "\n";
  // Translating the function body
  out << "{\n";
  // Translating local types if there are any
  if (local_types[fun_name_short].size() > 0)
  {
    out << "////////////////////   LOCAL TYPES  ///////////////////////\n";
    for (auto type : local_types[fun_name_short])
      out << typedef2c(type, ns) << "; // " << type.location() << "\n";
  }
  // Translating local static variables if there are any
  if (local_static_vars[fun_name_short].size() > 0)
  {
    out << "///////////////   LOCAL STATIC VARIABLES /////////////////\n";
    for (auto s : local_static_vars[fun_name_short])
    {
      out << expr2c(
               code_declt(symbol_expr(s), initializers[s.id.as_string()]), ns)
          << "; // " << s.location << "\n";
    }
  }
  // Translating the rest of the function body
  out << translate(goto_function.body);
  out << "}\n";
  return out.str();
}

// Convert GOTO program to C
std::string goto2ct::translate(goto_programt &goto_program)
{
  std::ostringstream out;

  std::vector<unsigned int> scope_ids_stack = {1};
  unsigned int cur_scope_id = scope_ids_stack.back();
  for (auto it = goto_program.instructions.begin();
       it != goto_program.instructions.end();
       it++)
  {
    // Scope change
    if (it->scope_id != cur_scope_id)
    {
      // Entering a new scope
      if (it->parent_scope_id == cur_scope_id)
      {
        out << "{ // SCOPE BEGIN {" << cur_scope_id << "}->{" << it->scope_id
            << "}\n";
        scope_ids_stack.push_back(it->scope_id);
        cur_scope_id = scope_ids_stack.back();
      }
      // Leaving the scope back to the current parent scope
      else
      {
        scope_ids_stack.pop_back();
        out << "} // SCOPE END {" << cur_scope_id << "}->{"
            << scope_ids_stack.back() << "}\n";
        cur_scope_id = scope_ids_stack.back();
        // If there two scopes next to each other (i.e., {inst1;}{inst2;})
        // we need to open a new scope immediately after the previous one
        // so that we do not skip through the first instruction
        // in the new scope
        if (it->scope_id != cur_scope_id)
        {
          out << "{ // SCOPE BEGIN {" << cur_scope_id << "}->{" << it->scope_id
              << "}\n";
          scope_ids_stack.push_back(it->scope_id);
          cur_scope_id = scope_ids_stack.back();
        }
      }
    }
    out << translate(*it) << "\n";
  }
  return out.str();
}

// Convert GOTO instruction to C
std::string goto2ct::translate(goto_programt::instructiont &instruction)
{
  std::ostringstream out;

  // This is a GOTO label
  if (instruction.is_target())
    out << "__ESBMC_goto_label_" << instruction.target_number
        << ":; // Target\n";

  // Identifying the type of the GOTO instruction
  switch (instruction.type)
  {
  case ASSERT:
    out << expr2c(code_assertt(migrate_expr_back(instruction.guard)), ns);
    break;
  case ASSUME:
    out << "// "
        << expr2c(code_assumet(migrate_expr_back(instruction.guard)), ns);
    break;
  case GOTO:
    // If we know that the guard is TRUE, then just skip the "if" part
    if (!is_true(instruction.guard))
      out << "if(" << expr2c(migrate_expr_back(instruction.guard), ns) << ") ";
    // It is guaranteed that every GOTO instruction has only one target
    out << "goto __ESBMC_goto_label_"
        << instruction.targets.front()->target_number;
    break;
  // These expressions can be just translated directly
  case FUNCTION_CALL:
  case RETURN:
  case DECL:
  case ASSIGN:
  case OTHER:
    out << expr2c(migrate_expr_back(instruction.code), ns);
    break;
  // "Silent" instructions. They cannot be translated directly into C.
  // So we just keep them for debugging purposes.
  case END_FUNCTION:
    out << "// " << instruction.location.function();
    break;
  case DEAD:
    out << "// " << expr2c(migrate_expr_back(instruction.code), ns);
    break;
  case LOCATION:
    out << "// " << instruction.location;
    break;
  case SKIP:
    out << "// " << instruction.location;
    break;
  case NO_INSTRUCTION_TYPE:
    out << "// " << instruction.location << "";
    break;
  // These are C++ instructions. We do not translate those at the moment.
  case THROW:
  case CATCH:
  case ATOMIC_BEGIN:
  case ATOMIC_END:
  case THROW_DECL:
  case THROW_DECL_END:
    assert(!"C++ instructions are not supported by GOTO2C");
    out << "C++ instruction";
    break;
  default:
    assert(!"Unknown instruction type");
    out << "unknown instruction";
  }
  out << ";";
  out << " // " << instruction.location.comment() << " // " << instruction.type;
  return out.str();
}
