# Generate GNU autoconf-style Fortran name mangling for ITAPS headers

# MACRO ( itaps_fortran_mangle input_file output_file prefix )

SET( match_expr "^[ \\t]*void[ \\t]+${prefix}_([a-zA-Z][_a-zA-Z0-9]*)[ \\t]*\\(.*$" )
FILE( STRINGS ${input_file} data REGEX ${match_expr} )
FILE( WRITE ${output_file} "#include \"MOAB_FCDefs.h\"\n#ifdef MOAB_FC_FUNC_\n\n" )
FOREACH( line ${data} )
  STRING(REGEX REPLACE ${match_expr} "${prefix}_\\1" func ${line})
  STRING(TOUPPER ${func} upper)
  STRING(TOLOWER ${func} lower)
  FILE( APPEND ${output_file}  "#define ${func} ${DEF}_FC_FUNC_( ${lower}, ${upper} )\n" )
ENDFOREACH( line )
FILE( APPEND ${output_file} "\n#endif\n" )

# ENDMACRO( itaps_fortran_mangle )
