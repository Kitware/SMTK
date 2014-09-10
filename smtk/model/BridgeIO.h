#ifndef __smtk_model_BridgeIO_h
#define __smtk_model_BridgeIO_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/util/SharedFromThis.h"

namespace smtk {
  namespace model {

/**\brief A base class for delegating bridge I/O.
  *
  * Bridges sometimes need to import and export additional
  * state beyond a list of operations.
  * For example, when it is not possible to store SMTK UUIDs
  * as modeling-kernel-native attributes, the bridge may
  * keep an array of UUIDs in order of iteration.
  *
  * Bridges should not have methods tied to any particular
  * format (e.g., JSON, XML, etc.); so we provide this
  * base class.
  * To use this class:
  *
  * + format-and-bridge-specific subclasses of this base
  *   class should be written; and
  * + the bridge subclass should override its
  *   createIODelegate() method to return instances of
  *   the above as required.
  *
  * Note that the I/O format being handled will dictate
  * additional subclass methods to be implemented.
  * For example, ImportJSON and ExportJSON will dynamically
  * cast the returned shared pointer to a BridgeIOJSON
  * instance and (if it is non-null) call methods which
  * accept cJSON pointers.
  */
class SMTKCORE_EXPORT BridgeIO
{
public:
  smtkTypeMacro(BridgeIO);
  virtual ~BridgeIO() { } // virtual method needed so dynamic_cast will work
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BridgeIO_h
