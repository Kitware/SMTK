//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SessionRef_h
#define __smtk_model_SessionRef_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/model/EntityRefArrangementOps.h" // for templated methods
#include "smtk/model/Model.h"

namespace smtk
{
namespace model
{

class SessionRef;
typedef std::vector<SessionRef> SessionRefs;

/**\brief A entityref subclass that provides methods specific to entity use records.
  *
  */
class SMTKCORE_EXPORT SessionRef : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(SessionRef, EntityRef, isSessionRef);
  SessionRef(ManagerPtr manager, SessionPtr brdg);

  SessionPtr session() const;

  template <typename T>
  T models() const;
  SessionRef& addModel(const Model& mod);

  std::string tag() const;
  std::string site() const;
  StringList engines() const;
  StringData fileTypes(const std::string& engine = std::string()) const;
  std::string defaultFileExtension(const Model& model = Model()) const;

  void close();
};

template <typename T>
T SessionRef::models() const
{
  return this->relationsAs<T>();
}
} // namespace model
} // namespace smtk

#endif // __smtk_model_SessionRef_h
