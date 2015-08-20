// Takes a DagMC-style meshset of quads and converts it to triangles.
// It is assumed that quads are only in surface meshsets. Meshset
// membership for tris is only preserved for surfaces meshsets of their
// parent quads.
//

#include <iostream>
#include <assert.h>
#include "moab/Core.hpp"
#include "MBTagConventions.hpp"
#include "moab/Range.hpp"

moab::ErrorCode make_tris_from_quad( moab::Interface *MBI,
                                 moab::EntityHandle quad,  /* input  */
                                 moab::EntityHandle &tri0, /* output */
				 moab::EntityHandle &tri1  /* output */);

moab::ErrorCode make_tris_from_quads( moab::Interface *MBI, const moab::Range quads, moab::Range &tris ) ;

moab::ErrorCode quads_to_tris( moab::Interface *MBI, moab::EntityHandle input_meshset );
