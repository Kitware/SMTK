//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionIO_h
#define __smtk_model_SessionIO_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

namespace smtk {
  namespace model {

/**\brief A base class for delegating session I/O.
  *
  * Sessions sometimes need to import and export additional
  * state beyond a list of operations.
  * For example, when it is not possible to store SMTK UUIDs
  * as modeling-kernel-native attributes, the session may
  * keep an array of UUIDs in order of iteration.
  *
  * Sessions should not have methods tied to any particular
  * format (e.g., JSON, XML, etc.); so we provide this
  * base class.
  * To use this class:
  *
  * + format-and-session-specific subclasses of this base
  *   class should be written; and
  * + the session subclass should override its
  *   createIODelegate() method to return instances of
  *   the above as required.
  *
  * Note that the I/O format being handled will dictate
  * additional subclass methods to be implemented.
  * For example, ImportJSON and ExportJSON will dynamically
  * cast the returned shared pointer to a SessionIOJSON
  * instance and (if it is non-null) call methods which
  * accept cJSON pointers.
  */
class SMTKCORE_EXPORT SessionIO
{
public:
  smtkTypeMacro(SessionIO);
  virtual ~SessionIO() { } // virtual method needed so dynamic_cast will work
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_SessionIO_h
