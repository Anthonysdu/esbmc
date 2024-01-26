#include <clang-cpp-frontend/esbmc_internal_cpp.h>
#include <util/filesystem.h>
extern "C"
{
#include <abstract_includes/cpp_includes.h>
#undef ESBMC_FLAIL
}

const std::string &esbmct::abstract_cpp_includes()
{
  // Dump CPP headers into a temporary directory
  static bool dumped = false;
  /* About the path being static:
   * The static member 'dumped' above is used to check whether the headers were
   * ever extracted before. This guarantees that the same path is used
   * during a run. And no more than one is required anyway */
  static auto p =
    file_operations::create_tmp_dir("esbmc-cpp-headers-%%%%-%%%%-%%%%");
  if (!dumped)
  {
    dumped = true;
#define ESBMC_FLAIL(body, size, ...)                                           \
  file_operations::create_path_and_write(                                      \
    p.path() + "/" #__VA_ARGS__, body, size);
#include <abstract_includes/cpp_includes.h> /* generated by build system */
#undef ESBMC_FLAIL
  }
  return p.path();
}