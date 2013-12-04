#include "smtk/util/UUID.h"
#include "smtk/model/Storage.h"
#include "smtk/model/testing/helpers.h"

#include <iomanip>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h> // for GetTickCount()
#else
#  include <sys/time.h> // for gettimeofday()
#  define SMTK_HAVE_GETTIMEOFDAY
#endif

using namespace smtk::util;
using namespace smtk::model;

namespace smtk {
  namespace model {
    namespace testing {

UUIDArray createTet(smtk::model::StoragePtr sm)
{
  static const double x[][3] = {
      { 0., 0., 0. },
      { 4., 0., 0. },
      { 2., 4., 0. },
      { 1., 1., 0. },
      { 2., 3., 0. },
      { 3., 1., 0. },
      { 2., 0.,-4. },
  };

  smtk::util::UUID uc00 = sm->insertCellOfDimension(0)->first; // keep just the UUID around.
  smtk::util::UUID uc01 = sm->insertCellOfDimension(0)->first;
  smtk::util::UUID uc02 = sm->insertCellOfDimension(0)->first;
  smtk::util::UUID uc03 = sm->insertCellOfDimension(0)->first;
  smtk::util::UUID uc04 = sm->insertCellOfDimension(0)->first;
  smtk::util::UUID uc05 = sm->insertCellOfDimension(0)->first;
  smtk::util::UUID uc06 = sm->insertCellOfDimension(0)->first;

  smtk::util::UUID uc07 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc01))->first;
  smtk::util::UUID uc08 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc02))->first;
  smtk::util::UUID uc09 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc00))->first;
  smtk::util::UUID uc10 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc03).appendRelation(uc04))->first;
  smtk::util::UUID uc11 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc04).appendRelation(uc05))->first;
  smtk::util::UUID uc12 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc05).appendRelation(uc03))->first;
  smtk::util::UUID uc13 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc06))->first;
  smtk::util::UUID uc14 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc06))->first;
  smtk::util::UUID uc15 = sm->insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc06))->first;

  smtk::util::UUID uc16 = sm->insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc08)
    .appendRelation(uc09)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  smtk::util::UUID uc17 = sm->insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  smtk::util::UUID uc18 = sm->insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc13)
    .appendRelation(uc14)
    )->first;
  smtk::util::UUID uc19 = sm->insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc08)
    .appendRelation(uc14)
    .appendRelation(uc15)
    )->first;
  smtk::util::UUID uc20 = sm->insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc09)
    .appendRelation(uc15)
    .appendRelation(uc13)
    )->first;

  smtk::util::UUID uc21 = sm->insertEntity(
    Entity(CELL_ENTITY, 3)
    .appendRelation(uc16)
    .appendRelation(uc17)
    .appendRelation(uc18)
    .appendRelation(uc19)
    .appendRelation(uc20))->first;

  sm->setTessellation(uc21, Tessellation()
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
    sm->setTessellation(uids[i],Tessellation().addCoords(x[i][0], x[i][1], x[i][2]));
    }

  return uids;
}

std::ostream& operator << (std::ostream& os, const hexconst& x)
{
  os << std::setbase(16) << std::fixed << std::setw(8) << std::setfill('0') << x.m_val;
  return os;
}

#ifdef SMTK_HAVE_GETTIMEOFDAY
class Timer::Internal
{
public:
  struct timeval m_mark;
  Internal()
    {
    this->fetch();
    }
  void fetch()
    {
    gettimeofday(&this->m_mark, NULL);
    }
};
#else
class Timer::Internal
{
public:
  unsigned long long m_mark;
  Internal()
    {
    this->fetch();
    }
  void fetch()
    {
    m_mark = GetTickCount64();
    }
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

void Timer::mark()
{
  this->P->fetch();
}

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
