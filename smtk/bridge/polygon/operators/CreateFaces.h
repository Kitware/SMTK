#ifndef __smtk_session_polygon_CreateFaces_h
#define __smtk_session_polygon_CreateFaces_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create a face given a set of point coordinates or edges (but not both).
  *
  */
class SMTKPOLYGONSESSION_EXPORT CreateFaces : public Operator
{
public:
  smtkTypeMacro(CreateFaces);
  smtkCreateMacro(CreateFaces);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateFaces_h
