file(READ "${input}" irel)
set(FPFX "iRel")

set(contents "#include \"iRel_FCDefs.h\"\n#ifdef IREL_FC_FUNC_\n\n")

string(REGEX MATCHALL "^void ${FPFX}_[a-z][_a-zA-Z0-9]*\\($" irel_functions "${irel}")
foreach (func IN LISTS irel_functions)
  string(REGEX MATCH "${FPFX}_([a-z][_a-zA-Z0-9]*)" "\\1" func_name "${func}")
  string(TOUPPER "${func_name}" upper)
  string(TOLOWER "${func_name}" lower)

  set(contents "${contents}#define ${func_name} IREL_FC_FUNC_( ${lower}, ${upper} )\n")
endforeach ()

set(contents "${contents}\n#endif\n")

file(WRITE "${output}" "${contents}")
