#ifndef __smtk_session_polygon_CreateModel_h
#define __smtk_session_polygon_CreateModel_h

#include "smtk/bridge/polygon/Operator.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

/**\brief Create a face given height, major and minor radii, and a number of sides.
  *
  * The number of sides must be 3 or greater.
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
