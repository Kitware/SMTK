#ifndef __smtk_session_polygon_PointerDefs_h
#define __smtk_session_polygon_PointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"

namespace smtk {
  namespace bridge {
    namespace polygon {

class Session;
typedef smtk::shared_ptr< smtk::bridge::polygon::Session > SessionPtr;

    } // namespace polygon
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_PointerDefs_h
