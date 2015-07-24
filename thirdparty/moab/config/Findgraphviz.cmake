
if ( NOT WIN32 )

  find_package(PkgConfig)
  pkg_check_modules( graphviz ${REQUIRED} libgvc libagraph libcdt libgraph libpathplan )
  if ( graphviz_FOUND )
    set ( graphviz_INCLUDE_DIRECTORIES ${graphviz_INCLUDE_DIRS} )
  endif ( graphviz_FOUND )

endif ( NOT WIN32 )

find_path( graphviz_INCLUDE_DIRECTORIES
  NAMES gvc.h
  PATHS
    ${graphviz_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
)

find_library( graphviz_AGRAPH_LIBRARY
  NAMES agraph 
  PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
)
find_library( graphviz_GVC_LIBRARY
  NAMES gvc 
  PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
)
find_library( graphviz_CDT_LIBRARY
  NAMES cdt 
  PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
)
find_library( graphviz_GRAPH_LIBRARY
  NAMES graph 
  PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
)
find_library( graphviz_PATHPLAN_LIBRARY
  NAMES pathplan 
  PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
)
if ( graphviz_INCLUDE_DIRECTORIES AND graphviz_AGRAPH_LIBRARY AND
     graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
     graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )
  set ( graphviz_FOUND 1 )
  set ( graphviz_LIBRARIES
    "${graphviz_GVC_LIBRARY};${graphviz_AGRAPH_LIBRARY};${graphviz_GRAPH_LIBRARY};"
    "${graphviz_CDT_LIBRARY};${graphviz_PATHPLAN_LIBRARY}"
    CACHE FILEPATH "Libraries for graphviz" )
else ( graphviz_INCLUDE_DIRECTORIES AND graphviz_AGRAPH_LIBRARY AND
       graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
       graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )
  set ( graphviz_FOUND 0 )
endif ( graphviz_INCLUDE_DIRECTORIES AND graphviz_AGRAPH_LIBRARY AND
        graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
        graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )
