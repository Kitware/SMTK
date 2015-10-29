#ifndef __smtk_session_polygon_CreateModel_h
#define __smtk_session_polygon_CreateModel_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create a polygonal model made up of vertices, edges, and faces.
  *
  * The geometry in the model is all planar.
  * By default, points are assumed to lie in the x-y plane with an
  * origin of (0,0), but you may provide any base point and axes you prefer.
  *
  * Coordinates are discretized to integers; you must pass either a feature
  * size or a model scale to control how fine the approximation is.
  *
  * Each polygonal modeling session may have multiple models but no
  * geometric entities may be shared between them;
  * attempting to share points across different discretizations on different
  * projected planes would be error-prone at best.
  */
class SMTKPOLYGONSESSION_EXPORT CreateModel : public Operator
{
public:
  smtkTypeMacro(CreateModel);
  smtkCreateMacro(CreateModel);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

    } // namespace polygon
  } //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_CreateModel_h
