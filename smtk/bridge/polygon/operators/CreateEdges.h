#ifndef __smtk_session_polygon_CreateEdges_h
#define __smtk_session_polygon_CreateEdges_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create one or more edges given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT CreateEdges : public Operator
{
public:
  smtkTypeMacro(CreateEdges);
  smtkCreateMacro(CreateEdges);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateEdges_h
