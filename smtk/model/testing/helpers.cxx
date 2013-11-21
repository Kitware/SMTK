#include "smtk/util/UUID.h"
#include "smtk/model/Storage.h"
#include "smtk/model/testing/helpers.h"

#include <iomanip>

#include <sys/time.h>

using namespace smtk::util;
using namespace smtk::model;

namespace smtk {
  namespace model {
    namespace testing {

UUIDArray createTet(smtk::model::Storage& sm)
{
  UUID uc00 = sm.insertCellOfDimension(0)->first; // keep just the UUID around.
  UUID uc01 = sm.insertCellOfDimension(0)->first;
  UUID uc02 = sm.insertCellOfDimension(0)->first;
  UUID uc03 = sm.insertCellOfDimension(0)->first;
  UUID uc04 = sm.insertCellOfDimension(0)->first;
  UUID uc05 = sm.insertCellOfDimension(0)->first;
  UUID uc06 = sm.insertCellOfDimension(0)->first;

  UUID uc07 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc01))->first;
  UUID uc08 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc02))->first;
  UUID uc09 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc00))->first;
  UUID uc10 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc03).appendRelation(uc04))->first;
  UUID uc11 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc04).appendRelation(uc05))->first;
  UUID uc12 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc05).appendRelation(uc03))->first;
  UUID uc13 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc00).appendRelation(uc06))->first;
  UUID uc14 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc01).appendRelation(uc06))->first;
  UUID uc15 = sm.insertEntity(Entity(CELL_ENTITY, 1).appendRelation(uc02).appendRelation(uc06))->first;

  UUID uc16 = sm.insertEntity(
    Entity(CELL_ENTITY, 2)
    .appendRelation(uc07)
    .appendRelation(uc08)
    .appendRelation(uc09)
    .appendRelation(uc10)
    .appendRelation(uc11)
    .appendRelation(uc12)
    )->first;
  UUID uc17 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc10).appendRelation(uc11).appendRelation(uc12))->first;
  UUID uc18 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc07).appendRelation(uc13).appendRelation(uc14))->first;
  UUID uc19 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc08).appendRelation(uc14).appendRelation(uc15))->first;
  UUID uc20 = sm.insertEntity(Entity(CELL_ENTITY, 2).appendRelation(uc09).appendRelation(uc15).appendRelation(uc13))->first;

  UUID uc21 = sm.insertEntity(
    Entity(CELL_ENTITY, 3)
    .appendRelation(uc16)
    .appendRelation(uc17)
    .appendRelation(uc18)
    .appendRelation(uc19)
    .appendRelation(uc20))->first;

  sm.setTessellation(uc21, Tessellation()
    .addCoords(0., 0., 0.)
    .addCoords(4., 0., 0.)
    .addCoords(2., 4., 0.)
    .addCoords(1., 1., 0.)
    .addCoords(2., 3., 0.)
    .addCoords(3., 1., 0.)
    .addCoords(2., 0.,-4.)
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
  return uids;
}

std::ostream& operator << (std::ostream& os, const hexconst& x)
{
  os << std::setbase(16) << std::fixed << std::setw(8) << std::setfill('0') << x.m_val;
  return os;
}

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
  double result = other.m_mark.tv_sec - this->P->m_mark.tv_sec;
  result += 1e-6 * (other.m_mark.tv_usec - this->P->m_mark.tv_usec);
  return result;
}

    } // namespace testing
  } // namespace model
} // namespace smtk
