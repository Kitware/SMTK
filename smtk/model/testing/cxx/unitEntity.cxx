//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

#include "smtk/model/testing/cxx/helpers.h"
#include "smtk/model/IntegerData.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <sstream>

#include <assert.h>

using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::model::testing;

static const char* correct =
"0x00000101  vertex\n"
"0x00000102  edge\n"
"0x00000104  face\n"
"0x00000108  volume\n"
"0x00000110  spacetime\n"
"0x00000100  mixed-dimension cell\n"
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
"0x00000101  vertices\n"
"0x00000102  edges\n"
"0x00000104  faces\n"
"0x00000108  volumes\n"
"0x00000110  spacetimes\n"
"0x00000100  mixed-dimension cells\n"
"0x00000201  vertex uses\n"
"0x00000202  edge uses\n"
"0x00000204  face uses\n"
"0x00000403  chains\n"
"0x00000406  loops\n"
"0x0000040c  shells\n"
"0x00000801  groups (vertex entities)\n"
"0x00000802  groups (edge entities)\n"
"0x00000804  groups (face entities)\n"
"0x00000808  groups (volume entities)\n"
"0x00000809  groups (vertex,volume entities)\n"
"0x02000804  domain groups (face entities)\n"
"0x02000808  domain groups (volume entities)\n"
"0x01000804  boundary groups (face entities)\n"
"0x01000808  boundary groups (volume entities)\n"
"0x02000800  domain groups\n"
"0x01000800  boundary groups\n"
"0x00000101  vertex 0\n"
"0x00000102  edge 0\n"
"0x00000104  face 0\n"
"0x00000108  volume 0\n"
"0x00000201  vertex use 1\n"
"0x00000202  edge use 1\n"
"0x00000204  face use 1\n"
"0x00000403  chain 2\n"
"0x00000406  loop 2\n"
"0x0000040c  shell 2\n"
"0x00000801  group 3\n"
"0x00000802  group 4\n"
"0x00000804  group 5\n"
"0x00000808  group 6\n"
"0x00000809  group 7\n"
"0x02000804  domain group 3\n"
"0x02000808  domain group 4\n"
"0x01000804  boundary group 3\n"
"0x01000808  boundary group 4\n"
"0x02000800  domain group 5\n"
"0x01000800  boundary group 5\n"
;

void EntityNamesForForm(std::ostringstream& summaries, int form)
{
  summaries
    << "0x" << hexconst(CELL_0D) << "  " << Entity::flagSummary(CELL_0D, form) << "\n"
    << "0x" << hexconst(CELL_1D) << "  " << Entity::flagSummary(CELL_1D, form) << "\n"
    << "0x" << hexconst(CELL_2D) << "  " << Entity::flagSummary(CELL_2D, form) << "\n"
    << "0x" << hexconst(CELL_3D) << "  " << Entity::flagSummary(CELL_3D, form) << "\n"
    << "0x" << hexconst(CELL_ENTITY | DIMENSION_4) << "  " << Entity::flagSummary(CELL_ENTITY | DIMENSION_4, form) << "\n"
    << "0x" << hexconst(CELL_ENTITY) << "  " << Entity::flagSummary(CELL_ENTITY, form) << "\n"

    << "0x" << hexconst(USE_0D) << "  " << Entity::flagSummary(USE_0D, form) << "\n"
    << "0x" << hexconst(USE_1D) << "  " << Entity::flagSummary(USE_1D, form) << "\n"
    << "0x" << hexconst(USE_2D) << "  " << Entity::flagSummary(USE_2D, form) << "\n"

    << "0x" << hexconst(SHELL_0D) << "  " << Entity::flagSummary(SHELL_0D, form) << "\n"
    << "0x" << hexconst(SHELL_1D) << "  " << Entity::flagSummary(SHELL_1D, form) << "\n"
    << "0x" << hexconst(SHELL_2D) << "  " << Entity::flagSummary(SHELL_2D, form) << "\n"

    << "0x" << hexconst(GROUP_0D) << "  " << Entity::flagSummary(GROUP_0D, form) << "\n"
    << "0x" << hexconst(GROUP_1D) << "  " << Entity::flagSummary(GROUP_1D, form) << "\n"
    << "0x" << hexconst(GROUP_2D) << "  " << Entity::flagSummary(GROUP_2D, form) << "\n"
    << "0x" << hexconst(GROUP_3D) << "  " << Entity::flagSummary(GROUP_3D, form) << "\n"

    << "0x" << hexconst(GROUP_0D | GROUP_3D) << "  " << Entity::flagSummary(GROUP_0D | GROUP_3D, form) << "\n"

    << "0x" << hexconst(GROUP_2D | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_2D | MODEL_DOMAIN, form) << "\n"
    << "0x" << hexconst(GROUP_3D | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_3D | MODEL_DOMAIN, form) << "\n"
    << "0x" << hexconst(GROUP_2D | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_2D | MODEL_BOUNDARY, form) << "\n"
    << "0x" << hexconst(GROUP_3D | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_3D | MODEL_BOUNDARY, form) << "\n"
    << "0x" << hexconst(GROUP_ENTITY | MODEL_DOMAIN  ) << "  " << Entity::flagSummary(GROUP_ENTITY | MODEL_DOMAIN, form) << "\n"
    << "0x" << hexconst(GROUP_ENTITY | MODEL_BOUNDARY) << "  " << Entity::flagSummary(GROUP_ENTITY | MODEL_BOUNDARY, form) << "\n"
    ;
}

int TestEntitySummary()
{
  smtk::model::Integer cdata[] = {0, 0, 0, 0, 0, 0};
  smtk::model::IntegerList counters(cdata, cdata + sizeof(cdata)/sizeof(cdata[0]));
  std::ostringstream summaries;

  // test default constructor
  Entity blank;

  // Test setting flags on invalid entity
  blank.setEntityFlags(GROUP_ENTITY | PARTITION);
  assert((blank.entityFlags() & (GROUP_ENTITY | PARTITION)) == (GROUP_ENTITY | PARTITION));

  // Test setting flags on valid entity (disallowed, so no effect)
  blank.setEntityFlags(CELL_2D);
  assert((blank.entityFlags() & (GROUP_ENTITY | PARTITION)) == (GROUP_ENTITY | PARTITION));

  // Test changing only property bits on valid entity (should succeed)
  blank.setEntityFlags(GROUP_ENTITY);
  assert(!(blank.entityFlags() & PARTITION));

  // Test Entity::dimensionBits()
  Entity shellEnt(SHELL_ENTITY | DIMENSION_1 | DIMENSION_2, -1);
  assert(shellEnt.dimensionBits() == 6);

  // Test Entity::flagDescription() and model/instance bit flags
  assert(Entity::flagDescription(MODEL_ENTITY, 1) == "models");
  assert(Entity::flagDescription(INSTANCE_ENTITY, 0) == "instance");

  // --- now test singular and plural forms pretty exhaustively
  EntityNamesForForm(summaries, /*singular*/ 0);
  EntityNamesForForm(summaries, /*plural*/ 1);

  // --- now test defaultNameFromCounters

  summaries << "0x" << hexconst(CELL_0D) << "  " << Entity::defaultNameFromCounters(CELL_0D, counters) << "\n";
  summaries << "0x" << hexconst(CELL_1D) << "  " << Entity::defaultNameFromCounters(CELL_1D, counters) << "\n";
  summaries << "0x" << hexconst(CELL_2D) << "  " << Entity::defaultNameFromCounters(CELL_2D, counters) << "\n";
  summaries << "0x" << hexconst(CELL_3D) << "  " << Entity::defaultNameFromCounters(CELL_3D, counters) << "\n";

  summaries << "0x" << hexconst(USE_0D) << "  " << Entity::defaultNameFromCounters(USE_0D, counters) << "\n";
  summaries << "0x" << hexconst(USE_1D) << "  " << Entity::defaultNameFromCounters(USE_1D, counters) << "\n";
  summaries << "0x" << hexconst(USE_2D) << "  " << Entity::defaultNameFromCounters(USE_2D, counters) << "\n";

  summaries << "0x" << hexconst(SHELL_0D) << "  " << Entity::defaultNameFromCounters(SHELL_0D, counters) << "\n";
  summaries << "0x" << hexconst(SHELL_1D) << "  " << Entity::defaultNameFromCounters(SHELL_1D, counters) << "\n";
  summaries << "0x" << hexconst(SHELL_2D) << "  " << Entity::defaultNameFromCounters(SHELL_2D, counters) << "\n";

  summaries << "0x" << hexconst(GROUP_0D) << "  " << Entity::defaultNameFromCounters(GROUP_0D, counters) << "\n";
  summaries << "0x" << hexconst(GROUP_1D) << "  " << Entity::defaultNameFromCounters(GROUP_1D, counters) << "\n";
  summaries << "0x" << hexconst(GROUP_2D) << "  " << Entity::defaultNameFromCounters(GROUP_2D, counters) << "\n";
  summaries << "0x" << hexconst(GROUP_3D) << "  " << Entity::defaultNameFromCounters(GROUP_3D, counters) << "\n";

  summaries << "0x" << hexconst(GROUP_0D | GROUP_3D)
    << "  " << Entity::defaultNameFromCounters(GROUP_0D | GROUP_3D, counters) << "\n";

  summaries << "0x" << hexconst(GROUP_2D | MODEL_DOMAIN  )
    << "  " << Entity::defaultNameFromCounters(GROUP_2D | MODEL_DOMAIN  , counters) << "\n";
  summaries << "0x" << hexconst(GROUP_3D | MODEL_DOMAIN  )
    << "  " << Entity::defaultNameFromCounters(GROUP_3D | MODEL_DOMAIN  , counters) << "\n";
  summaries << "0x" << hexconst(GROUP_2D | MODEL_BOUNDARY)
    << "  " << Entity::defaultNameFromCounters(GROUP_2D | MODEL_BOUNDARY, counters) << "\n";
  summaries << "0x" << hexconst(GROUP_3D | MODEL_BOUNDARY)
    << "  " << Entity::defaultNameFromCounters(GROUP_3D | MODEL_BOUNDARY, counters) << "\n";
  summaries << "0x" << hexconst(GROUP_ENTITY | MODEL_DOMAIN  )
    << "  " << Entity::defaultNameFromCounters(GROUP_ENTITY | MODEL_DOMAIN  , counters) << "\n";
  summaries << "0x" << hexconst(GROUP_ENTITY | MODEL_BOUNDARY)
    << "  " << Entity::defaultNameFromCounters(GROUP_ENTITY | MODEL_BOUNDARY, counters) << "\n";

  bool ok = summaries.str() == correct;
  if (!ok)
    {
    std::cerr
      << summaries.str() << "\n"
      << "not equal to\n"
      << correct << "\n";
    }
  return ok ? 0 : 1;
}

int TestEntityIOSpecs()
{
  static struct {
    std::string name;
    BitFlags value;
  } testToValValues[] = {
    { "anydim",     smtk::model::ANY_DIMENSION },
    { "bdy",        smtk::model::MODEL_BOUNDARY },
    { "bridge",     smtk::model::BRIDGE_SESSION },
    { "cell",       smtk::model::CELL_ENTITY },
    { "chain",      smtk::model::CHAIN },
    { "closed",     smtk::model::CLOSED },
    { "cover",      smtk::model::COVER },
    { "domain",     smtk::model::MODEL_DOMAIN },
    { "edge",       smtk::model::EDGE },
    { "edge_use",   smtk::model::EDGE_USE },
    { "face",       smtk::model::FACE },
    { "face_use",   smtk::model::FACE_USE },
    { "flat",       smtk::model::NO_SUBGROUPS },
    { "group",      smtk::model::GROUP_ENTITY },
    { "homg",       smtk::model::HOMOGENOUS_GROUP },
    { "instance",   smtk::model::INSTANCE_ENTITY },
    { "invalid",    smtk::model::INVALID },
    { "loop",       smtk::model::LOOP },
    { "model",      smtk::model::MODEL_ENTITY },
    { "nodim",      0 },
    { "none",       0 },
    { "open",       smtk::model::OPEN },
    { "partition",  smtk::model::PARTITION },
    { "shell",      smtk::model::SHELL_ENTITY },
    { "shell2",     smtk::model::SHELL },
    { "use",        smtk::model::USE_ENTITY },
    { "vertex",     smtk::model::VERTEX },
    { "vertex_use", smtk::model::VERTEX_USE },
    { "volume",     smtk::model::VOLUME },
    { "volume_use", smtk::model::VOLUME_USE },
    { "none|0",     smtk::model::DIMENSION_0 },
    { "none|1",     smtk::model::DIMENSION_1 },
    { "none|2",     smtk::model::DIMENSION_2 },
    { "none|3",     smtk::model::DIMENSION_3 },
    { "none|4",     smtk::model::DIMENSION_4 },
    { "none|410",   smtk::model::DIMENSION_0 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_4 },
    { "none|104",   smtk::model::DIMENSION_0 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_4 },
    { "none|12",    smtk::model::DIMENSION_1 | smtk::model::DIMENSION_2 },
    { "cell|0",     smtk::model::VERTEX },
    { "cell|1",     smtk::model::EDGE },
    { "cell|2",     smtk::model::FACE },
    { "cell|3",     smtk::model::VOLUME },
    { "||3",        smtk::model::DIMENSION_3 },
    { "|3|",        smtk::model::DIMENSION_3 },
    { "none||",     0 },
  };
  static int numTestToValValues = sizeof(testToValValues) / sizeof(testToValValues[0]);
  std::cout << "\nTesting Entity::specifierStringToFlag()\n\n";
  for (int i = 0; i < numTestToValValues; ++i)
    {
    std::ostringstream msg;
    BitFlags expected = testToValValues[i].value;
    std::string testValue = testToValValues[i].name;
    BitFlags result = Entity::specifierStringToFlag(testValue);
    msg << "(" << testValue << ") -> \"" << std::ios_base::hex << result << " expected " << std::ios_base::hex << expected;
    test(result == expected, msg.str());
    }

  static struct {
    std::string name;
    BitFlags value;
  } testToSpecValues[] = {
    { "none|anydim",           smtk::model::ANY_DIMENSION },
    { "none|bdy|nodim",        smtk::model::MODEL_BOUNDARY },
    { "bridge|nodim",          smtk::model::BRIDGE_SESSION },
    { "cell|nodim",            smtk::model::CELL_ENTITY },
    { "cell|anydim",           smtk::model::CELL_ENTITY | smtk::model::ANY_DIMENSION },
    { "chain",                 smtk::model::CHAIN },
    { "none|closed|nodim",     smtk::model::CLOSED },
    { "none|cover|nodim",      smtk::model::COVER },
    { "none|domain|nodim",     smtk::model::MODEL_DOMAIN },
    { "edge",                  smtk::model::EDGE },
    { "edge_use",              smtk::model::EDGE_USE },
    { "face",                  smtk::model::FACE },
    { "face_use",              smtk::model::FACE_USE },
    { "none|flat|nodim",       smtk::model::NO_SUBGROUPS },
    { "group|nodim",           smtk::model::GROUP_ENTITY },
    { "none|homg|nodim",       smtk::model::HOMOGENOUS_GROUP },
    { "instance|nodim",        smtk::model::INSTANCE_ENTITY },
    { "invalid",               smtk::model::INVALID },
    { "loop",                  smtk::model::LOOP },
    { "model|nodim",           smtk::model::MODEL_ENTITY },
    { "none|nodim",            0 },
    { "none|open|nodim",       smtk::model::OPEN },
    { "none|partition|nodim",  smtk::model::PARTITION },
    { "shell|nodim",           smtk::model::SHELL_ENTITY },
    { "shell2",                smtk::model::SHELL },
    { "use|nodim",             smtk::model::USE_ENTITY },
    { "vertex",                smtk::model::VERTEX },
    { "vertex_use",            smtk::model::VERTEX_USE },
    { "volume",                smtk::model::VOLUME },
    { "volume_use",            smtk::model::VOLUME_USE },
    { "none|0",                smtk::model::DIMENSION_0 },
    { "none|1",                smtk::model::DIMENSION_1 },
    { "none|2",                smtk::model::DIMENSION_2 },
    { "none|3",                smtk::model::DIMENSION_3 },
    { "none|4",                smtk::model::DIMENSION_4 },
    { "none|014",              smtk::model::DIMENSION_0 | smtk::model::DIMENSION_1 | smtk::model::DIMENSION_4 },
    { "cell|open|0",           smtk::model::VERTEX | smtk::model::OPEN },
  };
  static int numTestToSpecValues = sizeof(testToSpecValues) / sizeof(testToSpecValues[0]);
  std::cout << "\nTesting Entity::flagToSpecifierString()\n\n";
  for (int i = 0; i < numTestToSpecValues; ++i)
    {
    std::ostringstream msg;
    std::string expected = testToSpecValues[i].name;
    BitFlags testValue = testToSpecValues[i].value;
    std::string result = Entity::flagToSpecifierString(testValue);
    msg << "(" << std::ios_base::hex << testValue << ") -> \"" << result << "\" expected \"" << expected << "\"";
    test(result == expected, msg.str());
    }
  return 0;
}

int main()
{
  int status = 0;
  try
    {
    status |= TestEntityIOSpecs();
    }
  catch (const std::string& msg)
    {
    status = 1;
    }

  status |= TestEntitySummary();

  return status ? 1 : 0;
}
