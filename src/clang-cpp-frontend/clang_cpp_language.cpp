#include <util/compiler_defs.h>
// Remove warnings from Clang headers
CC_DIAGNOSTIC_PUSH()
CC_DIAGNOSTIC_IGNORE_LLVM_CHECKS()
#include <clang/Frontend/ASTUnit.h>
CC_DIAGNOSTIC_POP()

#include <util/c_link.h>
#include <c2goto/cprover_library.h>
#include <clang-c-frontend/clang_c_main.h>
#include <clang-cpp-frontend/clang_cpp_adjust.h>
#include <clang-cpp-frontend/clang_cpp_convert.h>
#include <clang-cpp-frontend/clang_cpp_language.h>
#include <clang-cpp-frontend/expr2cpp.h>
#include <regex>
#include <util/show_symbol_table.h>
#include <iostream>

languaget *new_clang_cpp_language()
{
  return new clang_cpp_languaget;
}

void clang_cpp_languaget::force_file_type()
{
  // C++ standard
  std::string cppstd = config.options.get_option("cppstd");
  if(!cppstd.empty())
  {
    auto it = std::find(standards.begin(), standards.end(), cppstd);
    if(it == standards.end())
    {
      log_error("Invalid C++ standard: {}", cppstd);
      abort();
    }
  }

  std::string clangstd = cppstd.empty() ? "-std=c++03" : "-std=c++" + cppstd;
  compiler_args.push_back(clangstd);

  // Force clang see all files as .cpp
  compiler_args.push_back("-x");
  compiler_args.push_back("c++");
}

std::string clang_cpp_languaget::internal_additions()
{
  std::string intrinsics = "extern \"C\" {\n";
  intrinsics.append(clang_c_languaget::internal_additions());
  intrinsics.append("}\n");

  // Replace _Bool by bool and return
  return std::regex_replace(intrinsics, std::regex("_Bool"), "bool");
}

bool clang_cpp_languaget::typecheck(
  contextt &context,
  const std::string &module)
{
  contextt new_context;

  clang_cpp_convertert converter(new_context, ASTs, "C++");
  if(converter.convert())
    return true;

#if 0
  // DEBUG: before adjustment
  printf("@@ before adjustment\n");
  std::ostringstream oss;
  ::show_symbol_table_plain(namespacet(new_context), oss);
  std::cout << oss.str() << std::endl;
#endif

  clang_cpp_adjust adjuster(new_context);
  if(adjuster.adjust())
    return true;

#if 0
  // DEBUG: after adjustment
  printf("@@ after adjustment\n");
  std::ostringstream oss_out2;
  ::show_symbol_table_plain(namespacet(new_context), oss_out2);
  std::cout << oss_out2.str() << std::endl;
#endif

  if(c_link(context, new_context, module))
    return true;

  return false;
}

bool clang_cpp_languaget::final(contextt &context)
{
  add_cprover_library(context);
  clang_c_maint c_main(context);
  return c_main.clang_c_main();
}

bool clang_cpp_languaget::from_expr(
  const exprt &expr,
  std::string &code,
  const namespacet &ns)
{
  code = expr2cpp(expr, ns);
  return false;
}

bool clang_cpp_languaget::from_type(
  const typet &type,
  std::string &code,
  const namespacet &ns)
{
  code = type2cpp(type, ns);
  return false;
}
