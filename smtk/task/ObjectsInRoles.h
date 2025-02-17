//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_ObjectsInRoles_h
#define smtk_task_ObjectsInRoles_h

#include "smtk/task/PortData.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace task
{

/**\brief ObjectsInRoles holds references to objects as its payload.
  *
  */
class SMTKCORE_EXPORT ObjectsInRoles : public PortData
{
public:
  smtkTypeMacro(smtk::task::ObjectsInRoles);
  smtkSuperclassMacro(smtk::task::PortData);
  smtkCreateMacro(smtk::task::PortData);

  using ObjectSet = std::unordered_set<smtk::resource::PersistentObject*>;
  using RoleMap = std::unordered_map<smtk::string::Token, ObjectSet>;

  /// Return the objects for this port.
  const RoleMap& data() const { return m_data; }

  /// Add an \a object to this data in the specified \a role.
  ///
  /// Multiple objects may be added to any role.
  /// You may not add an object with an invalid role (i.e., `!role.valid()`);
  /// if you intend to add objects with no role assigned, use a token such as "unassigned".
  bool addObject(smtk::resource::PersistentObject* object, smtk::string::Token role);

  /// Remove an object from this data.
  ///
  /// If a \a role is provided, the object will only be removed from that role.
  /// If no role is provided, all occurrences of the object will be removed.
  bool removeObject(
    smtk::resource::PersistentObject* object,
    smtk::string::Token role = smtk::string::Token::Invalid);

  /// Reset all the data in the role map.
  bool clear();

  bool merge(const PortData* other) override;

protected:
  RoleMap m_data;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_ObjectsInRoles_h
