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
struct Tessellation
{
  std::vector<double> Coords;
  std::vector<int> Conn;
  // We may eventually want geometry to include a reference
  // to a: boost::variant<point,curve,face,volume> Definition;

  Tessellation();

  int AddCoords(double* a);
  Tessellation& AddCoords(double x, double y, double z);

  Tessellation& AddPoint(double* a);
  Tessellation& AddLine(double* a, double* b);
  Tessellation& AddTriangle(double* a, double* b, double* c);

  Tessellation& AddPoint(int ai);
  Tessellation& AddLine(int ai, int bi);
  Tessellation& AddTriangle(int ai, int bi, int ci);

  Tessellation& Reset();
};

typedef std::map<smtk::util::UUID,Tessellation> UUIDsToTessellations;

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Tessellation_h
