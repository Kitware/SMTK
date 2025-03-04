//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/UUID.h"
#include "smtk/model/Chain.h"
#include "smtk/model/Edge.h"
#include "smtk/model/EdgeUse.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Shell.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/VertexUse.h"
#include "smtk/model/Volume.h"

#include "smtk/model/testing/cxx/helpers.h"

#include <iomanip>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h> // for GetTickCount()
#else
#include <sys/time.h> // for gettimeofday()
#define SMTK_HAVE_GETTIMEOFDAY
#endif

using namespace smtk::common;
using namespace smtk::model;
using smtk::common::UUID;
using smtk::common::UUIDArray;

namespace smtk
{
namespace model
{
namespace testing
{

UUIDArray createTet(smtk::model::ResourcePtr sm)
{
  static const double x[][3] = {
    { 0., 0., 0. }, { 4., 0., 0. }, { 2., 4., 0. },  { 1., 1., 0. },
    { 2., 3., 0. }, { 3., 1., 0. }, { 2., 0., -4. },
  };

  UUID uc00 = sm->insertCellOfDimension(0)->first; // keep just the UUID around.
  UUID uc01 = sm->insertCellOfDimension(0)->first;
  UUID uc02 = sm->insertCellOfDimension(0)->first;
  UUID uc03 = sm->insertCellOfDimension(0)->first;
  UUID uc04 = sm->insertCellOfDimension(0)->first;
  UUID uc05 = sm->insertCellOfDimension(0)->first;
  UUID uc06 = sm->insertCellOfDimension(0)->first;

  UUID uc07 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc00)->pushRelation(uc01))->first;
  UUID uc08 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc01)->pushRelation(uc02))->first;
  UUID uc09 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc02)->pushRelation(uc00))->first;
  UUID uc10 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc03)->pushRelation(uc04))->first;
  UUID uc11 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc04)->pushRelation(uc05))->first;
  UUID uc12 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc05)->pushRelation(uc03))->first;
  UUID uc13 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc00)->pushRelation(uc06))->first;
  UUID uc14 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc01)->pushRelation(uc06))->first;
  UUID uc15 =
    sm->insertEntity(Entity::create(CELL_ENTITY, 1)->pushRelation(uc02)->pushRelation(uc06))->first;

  UUID uc16 = sm->insertEntity(Entity::create(CELL_ENTITY, 2)
                                 ->pushRelation(uc07)
                                 ->pushRelation(uc08)
                                 ->pushRelation(uc09)
                                 ->pushRelation(uc10)
                                 ->pushRelation(uc11)
                                 ->pushRelation(uc12))
                ->first;
  UUID uc17 =
    sm->insertEntity(
        Entity::create(CELL_ENTITY, 2)->pushRelation(uc10)->pushRelation(uc12)->pushRelation(uc11))
      ->first;
  UUID uc18 =
    sm->insertEntity(
        Entity::create(CELL_ENTITY, 2)->pushRelation(uc07)->pushRelation(uc13)->pushRelation(uc14))
      ->first;
  UUID uc19 =
    sm->insertEntity(
        Entity::create(CELL_ENTITY, 2)->pushRelation(uc08)->pushRelation(uc14)->pushRelation(uc15))
      ->first;
  UUID uc20 =
    sm->insertEntity(
        Entity::create(CELL_ENTITY, 2)->pushRelation(uc09)->pushRelation(uc15)->pushRelation(uc13))
      ->first;

  UUID uc21 = sm->insertEntity(Entity::create(CELL_ENTITY, 3)
                                 ->pushRelation(uc16)
                                 ->pushRelation(uc17)
                                 ->pushRelation(uc18)
                                 ->pushRelation(uc19)
                                 ->pushRelation(uc20))
                ->first;

  sm->setTessellationAndBoundingBox(
    uc21,
    Tessellation()
      .addCoords(x[0][0], x[0][1], x[0][2])
      .addCoords(x[1][0], x[1][1], x[1][2])
      .addCoords(x[2][0], x[2][1], x[2][2])
      .addCoords(x[3][0], x[3][1], x[3][2])
      .addCoords(x[4][0], x[4][1], x[4][2])
      .addCoords(x[5][0], x[5][1], x[5][2])
      .addCoords(x[6][0], x[6][1], x[6][2])
      .addTriangle(0, 3, 5)
      .addTriangle(0, 5, 1)
      .addTriangle(1, 5, 4)
      .addTriangle(1, 4, 2)
      .addTriangle(2, 4, 3)
      .addTriangle(2, 3, 0)
      .addTriangle(3, 5, 4)
      .addTriangle(0, 6, 1)
      .addTriangle(1, 6, 2)
      .addTriangle(2, 6, 0));

  UUIDArray uids;
  uids.push_back(uc00);
  uids.push_back(uc01);
  uids.push_back(uc02);
  uids.push_back(uc03);
  uids.push_back(uc04);
  uids.push_back(uc05);
  uids.push_back(uc06);
  uids.push_back(uc07);
  uids.push_back(uc08);
  uids.push_back(uc09);
  uids.push_back(uc10);
  uids.push_back(uc11);
  uids.push_back(uc12);
  uids.push_back(uc13);
  uids.push_back(uc14);
  uids.push_back(uc15);
  uids.push_back(uc16);
  uids.push_back(uc17);
  uids.push_back(uc18);
  uids.push_back(uc19);
  uids.push_back(uc20);
  uids.push_back(uc21);

  // Add point coordinates
  for (int i = 0; i < 7; ++i)
  {
    sm->setTessellationAndBoundingBox(uids[i], Tessellation().addCoords(x[i][0], x[i][1], x[i][2]));
  }

  // Create vertex-uses
  // Because we have a single volume, each vertex should have a single use.
  VertexUses vu(7);
  for (int i = 0; i < 7; ++i)
  {
    vu[i] = sm->addVertexUse(
      /*vert*/ Vertex(sm, uids[i]),
      /*sense*/ 0);
    uids.push_back(vu[i].entity());
  }

  // Create 5 face-uses and shell for 1 volume.
  FaceUses su;
  for (int i = 0; i < 5; ++i)
  {
    su.push_back(sm->addFaceUse(
      /*face*/ Face(sm, uids[16 + i]),
      /*sense*/ 0,
      /*orientation*/ NEGATIVE));
    uids.push_back(su.back().entity());
  }
  Shell sh = sm->addShell(Volume(sm, uc21)).addUses(su);
  uids.push_back(sh.entity());

  // Create edge-uses and loops (6 of them) for 5 faces.
  EdgeUses lu[6];
  Loop lp[6];
  Chains chains;

  int ee[9][2] = {
    // Edge endpoints
    { 0, 1 }, { 1, 2 }, { 2, 0 }, { 3, 4 }, { 4, 5 }, { 5, 3 }, { 0, 6 }, { 1, 6 }, { 2, 6 },
  };
  int eul[6][3] = { // edge-use-to-loop uid offsets
                    { 9, 8, 7 },   { 10, 11, 12 }, { 12, 11, 10 },
                    { 7, 14, 13 }, { 8, 15, 14 },  { 9, 13, 15 }
  };
  int eus[6][3] = { // edge-use sense
                    { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, { 1, 1, 0 }, { 1, 1, 0 }, { 1, 1, 0 }
  };
  Orientation euo[6][3] = { // edge-use orientations
                            { NEGATIVE, NEGATIVE, NEGATIVE }, { NEGATIVE, NEGATIVE, NEGATIVE },
                            { POSITIVE, POSITIVE, POSITIVE }, { POSITIVE, POSITIVE, NEGATIVE },
                            { POSITIVE, POSITIVE, NEGATIVE }, { POSITIVE, POSITIVE, NEGATIVE }
  };
  for (int i = 0; i < 6; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      lu[i].push_back(sm->addEdgeUse(
        /*edge*/ Edge(sm, uids[eul[i][j]]),
        /*sense*/ eus[i][j],
        /*orientation*/ euo[i][j]));
      chains.emplace_back(sm->addChain(lu[i].back())
                            .addUse(vu[ee[eul[i][j] - 7][euo[i][j] == POSITIVE ? 0 : 1]])
                            .addUse(vu[ee[eul[i][j] - 7][euo[i][j] == POSITIVE ? 1 : 0]]));
      uids.push_back(lu[i][j].entity());
      uids.push_back(chains.back().entity());
    }
    // Add the loops, remembering that the second
    // loop is a hole and is thus a child of another loop:
    if (i == 0)
    {
      lp[i] = sm->addLoop(su[0]).addUses(lu[i]).as<Loop>();
    }
    else if (i == 1)
    {
      lp[i] = sm->addLoop(lp[0]).addUses(lu[i]).as<Loop>();
    }
    else
    { // note special index uids[15+i] b/c the hole-loop is repeated:
      lp[i] = sm->addLoop(su[i - 1]).addUses(lu[i]).as<Loop>();
    }
    uids.push_back(lp[i].entity());
  }

  return uids;
}

/** Report an integer as a hexadecimal value.
  *
  * The constant will be zero-padded to a width of 8.
  */
std::ostream& operator<<(std::ostream& os, const hexconst& x)
{
  os << std::setbase(16) << std::fixed << std::setw(8) << std::setfill('0') << x.m_val;
  return os;
}

/// Platform-specific data for measuring elapsed time.
#ifdef SMTK_HAVE_GETTIMEOFDAY
class Timer::Internal
{
public:
  struct timeval m_mark;
  Internal() { this->fetch(); }
  void fetch() { gettimeofday(&m_mark, nullptr); }
};
#else
class Timer::Internal
{
public:
  unsigned long long m_mark;
  Internal() { this->fetch(); }
  void fetch() { m_mark = GetTickCount64(); }
};
#endif

Timer::Timer()
{
  this->P = new Internal;
}

Timer::~Timer()
{
  delete this->P;
}

/// Mark a time as the start time for reporting elapsed time.
void Timer::mark()
{
  this->P->fetch();
}

/** Report the number of seconds since Timer::mark() was last called.
  *
  * On sane platforms, this will have microsecond precision.
  * On Windows, this will have millisecond precision at best.
  */
double Timer::elapsed()
{
  Internal other;
#ifdef SMTK_HAVE_GETTIMEOFDAY
  double result = other.m_mark.tv_sec - this->P->m_mark.tv_sec;
  result += 1e-6 * (other.m_mark.tv_usec - this->P->m_mark.tv_usec);
#else
  double result = 1e-3 * (other.m_mark - this->P->m_mark);
#endif
  return result;
}

} // namespace testing
} // namespace model
} // namespace smtk
