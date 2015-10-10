#ifndef __smtk_session_polygon_SplitEdge_h
#define __smtk_session_polygon_SplitEdge_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create one or more edges given a set of point coordinates.
  *
  * Self-intersecting edges are broken into multiple non-self-intersecting edges.
  */
class SMTKPOLYGONSESSION_EXPORT SplitEdge : public Operator
{
public:
  smtkTypeMacro(SplitEdge);
  smtkCreateMacro(SplitEdge);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_SplitEdge_h
