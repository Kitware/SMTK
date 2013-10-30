#ifndef __smtk_model_Tessellation_h
#define __smtk_model_Tessellation_h

#include "smtk/util/UUID.h"

#include <map>
#include <vector>

namespace smtk {
  namespace model {

/**\brief Store geometric information related to model entities.
  *
  * This is currently used to store coordinates and connectivity of
  * a triangulation of the model entity for rendering.
  * However, it may also evolve to store information about the
  * underlying geometric construct being approximated.
  */
struct SMTKCORE_EXPORT Tessellation
{
  std::vector<double> coords;
  std::vector<int> conn;
  // We may eventually want geometry to include a reference
  // to a: boost::variant<point,curve,face,volume> Definition;

  Tessellation();

  int addCoords(double* a);
  Tessellation& addCoords(double x, double y, double z);

  Tessellation& addPoint(double* a);
  Tessellation& addLine(double* a, double* b);
  Tessellation& addTriangle(double* a, double* b, double* c);

  Tessellation& addPoint(int ai);
  Tessellation& addLine(int ai, int bi);
  Tessellation& addTriangle(int ai, int bi, int ci);

  Tessellation& reset();
};

typedef std::map<smtk::util::UUID,Tessellation> UUIDsToTessellations;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Tessellation_h
