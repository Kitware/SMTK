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

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include <string>

namespace smtk
{
namespace model
{

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
  * For example, LoadJSON and SaveJSON will dynamically
  * cast the returned shared pointer to a SessionIOJSON
  * instance and (if it is non-null) call methods which
  * accept cJSON pointers.
  */
class SMTKCORE_EXPORT SessionIO
{
public:
  smtkTypeMacroBase(SessionIO);
  virtual ~SessionIO() {} // virtual method needed so dynamic_cast will work

  /**\brief Return a reference directory to use during import/export.
    *
    * Any relative file or directory specification should be
    * relative to this reference path. This is used during export to
    * turn absolute paths into relative ones. It may be used
    * during import to do the reverse.
    */
  std::string referencePath() const { return this->m_referencePath; }

  /// Set the directory to use during import/export for creating relative paths.
  void setReferencePath(const std::string& p) { this->m_referencePath = p; }

  /**\brief Save a resource file owned by the session for which this object delegates.
    */
  virtual int saveResource(
    const EntityRef& ent, smtk::common::ResourceSetPtr rset, StoredResourcePtr rsrc);

protected:
  std::string m_referencePath;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_SessionIO_h
