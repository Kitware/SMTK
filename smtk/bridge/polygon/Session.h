//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_polygon_Session_h
#define __smtk_session_polygon_Session_h

#include "smtk/bridge/polygon/Exports.h"
#include "smtk/bridge/polygon/PointerDefs.h"
#include "smtk/bridge/polygon/internal/Entity.h"

#include "smtk/model/Session.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{
namespace internal
{
class pmodel;
class vertex;
}

/**\brief Methods that handle translation between polygon and SMTK instances.
  *
  * While the TDUUID class keeps a map from SMTK UUIDs to polygon ToolDataUser
  * pointers, this is not enough to handle everything SMTK provides:
  * there is no way to track cell-use or shell entities since they do
  * not inherit ToolDataUser instances. Also, some engines (e.g., facet)
  * do not appear to store some entity types (e.g., RefGroup).
  *
  * Also, simply loading a polygon file does not translate the entire model
  * into SMTK; instead, it assigns UUIDs to entities if they do not already
  * exist. This class (Session) provides a method for requesting the
  * entity, arrangement, and/or tessellation information for a UUID be
  * mapped into SMTK from polygon.
  */
class SMTKPOLYGONSESSION_EXPORT Session : public smtk::model::Session
{
public:
  smtkTypeMacro(Session);
  smtkSuperclassMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkCreateMacro(smtk::model::Session);
  smtkDeclareModelingKernel();
  typedef smtk::model::SessionInfoBits SessionInfoBits;
  virtual ~Session();

  virtual SessionInfoBits allSupportedInformation() const;

  template <typename T, typename U, typename V>
  void consistentInternalDelete(T& container, U& modified, V& expunged, bool logDebug);

protected:
  friend class Neighborhood;
  friend class Operator;
  friend class internal::pmodel;
  friend class SessionIOJSON;

  Session();

  virtual smtk::model::SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity, SessionInfoBits requestedInfo, int depth = -1);

  void addStorage(
    const smtk::common::UUID& uid, smtk::bridge::polygon::internal::entity::Ptr storage);
  bool removeStorage(const smtk::common::UUID& uid);

  bool removeFaceReferences(const smtk::model::Face& f);
  bool removeEdgeReferences(const smtk::model::Edge& e);
  bool removeVertReferences(const smtk::model::Vertex& v);

  template <typename T>
  typename T::Ptr findStorage(const smtk::common::UUID& uid)
  {
    internal::EntityIdToPtr::iterator it = this->m_storage.find(uid);
    if (it != this->m_storage.end())
      return smtk::dynamic_pointer_cast<T>(it->second);
    static typename T::Ptr blank;
    return blank;
  }

  template <typename T>
  T findOrAddStorage(const smtk::common::UUID& uid)
  {
    internal::EntityIdToPtr::iterator it = this->m_storage.find(uid);
    if (it != this->m_storage.end())
      return smtk::dynamic_pointer_cast<T>(it->second);

    T blank = T::create();
    it = this->m_storage
           .insert(internal::EntityIdToPtr::value_type(
             uid, smtk::dynamic_pointer_cast<internal::entity>(blank)))
           .first;
    return smtk::dynamic_pointer_cast<T>(it->second);
  }

  virtual smtk::model::SessionIOPtr createIODelegate(const std::string& format);

  internal::EntityIdToPtr::iterator findStorageIterator(const smtk::common::UUID& uid);
  internal::EntityIdToPtr::iterator beginStorage();
  internal::EntityIdToPtr::iterator endStorage();
  internal::EntityIdToPtr::const_iterator beginStorage() const;
  internal::EntityIdToPtr::const_iterator endStorage() const;

  internal::EntityIdToPtr m_storage;
  int m_nextModelNumber;

private:
  Session(const Session&);        // Not implemented.
  void operator=(const Session&); // Not implemented.
};

} // namespace polygon
} //namespace bridge
} // namespace smtk

#endif // __smtk_session_polygon_Session_h
