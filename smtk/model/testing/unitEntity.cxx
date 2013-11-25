#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

#include "smtk/model/testing/helpers.h"
#include "smtk/model/ImportJSON.h"

#include <iostream>
#include <sstream>

#include <assert.h>

using namespace smtk::util;
using namespace smtk::model;
using namespace smtk::model::testing;

static const char* correct =
"0x00000101  vertex\n"
"0x00000102  edge\n"
"0x00000104  face\n"
"0x00000108  volume\n"
"0x00000201  vertex use\n"
"0x00000202  edge use\n"
"0x00000204  face use\n"
"0x00000403  chain\n"
"0x00000406  loop\n"
"0x0000040c  shell\n"
"0x00000801  group (vertex entities)\n"
"0x00000802  group (edge entities)\n"
"0x00000804  group (face entities)\n"
"0x00000808  group (volume entities)\n"
"0x00000809  group (vertex,volume entities)\n"
"0x02000804  domain group (face entities)\n"
"0x02000808  domain group (volume entities)\n"
"0x01000804  boundary group (face entities)\n"
"0x01000808  boundary group (volume entities)\n"
"0x02000800  domain group\n"
"0x01000800  boundary group\n"
;

int main()
{
  std::ostringstream summaries;
  summaries
    << "0x" << hexconst(CELL_0D) << "  " << Entity::flagSummary(CELL_0D) << "\n"
    << "0x" << hexconst(CELL_1D) << "  " << Entity::flagSummary(CELL_1D) << "\n"
    << "0x" << hexconst(CELL_2D) << "  " << Entity::flagSummary(CELL_2D) << "\n"
    << "0x" << hexconst(CELL_3D) << "  " << Entity::flagSummary(CELL_3D) << "\n"

    << "0x" << hexconst(USE_0D) << "  " << Entity::flagSummary(USE_0D) << "\n"
    << "0x" << hexconst(USE_1D) << "  " << Entity::flagSummary(USE_1D) << "\n"
    << "0x" << hexconst(USE_2D) << "  " << Entity::flagSummary(USE_2D) << "\n"

    << "0x" << hexconst(SHELL_0D) << "  " << Entity::flagSummary(SHELL_0D) << "\n"
    << "0x" << hexconst(SHELL_1D) << "  " << Entity::flagSummary(SHELL_1D) << "\n"
    << "0x" << hexconst(SHELL_2D) << "  " << Entity::flagSummary(SHELL_2D) << "\n"

    << "0x" << hexconst(GROUP_0D) << "  " << Entity::flagSummary(GROUP_0D) << "\n"
    << "0x" << hexconst(GROUP_1D) << "  " << Entity::flagSummary(GROUP_1D) << "\n"
    << "0x" << hexconst(GROUP_2D) << "  " << Entity::flagSummary(GROUP_2D) << "\n"
    << "0x" << hexconst(GROUP_3D) << "  " << Entity::flagSummary(GROUP_3D) << "\n"

    << "0x" << hexconst(GROUP_0D | GROUP_3D) << "  " << Entity::flagSummary(GROUP_0D | GROUP_3D) << "\n"

    << "0x" << hexconst(GROUP_2D | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_2D | MODEL_DOMAIN  ) << "\n"
    << "0x" << hexconst(GROUP_3D | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_3D | MODEL_DOMAIN  ) << "\n"
    << "0x" << hexconst(GROUP_2D | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_2D | MODEL_BOUNDARY) << "\n"
    << "0x" << hexconst(GROUP_3D | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_3D | MODEL_BOUNDARY) << "\n"
    << "0x" << hexconst(GROUP_ENTITY | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_ENTITY | MODEL_DOMAIN  ) << "\n"
    << "0x" << hexconst(GROUP_ENTITY | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_ENTITY | MODEL_BOUNDARY) << "\n"
    ;

  return summaries.str() == correct ? 0 : 1;
}
