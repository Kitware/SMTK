//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_CopyOptions_h
#define smtk_resource_CopyOptions_h

#include "smtk/resource/Links.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/common/TypeContainer.h"

#include <set>
#include <unordered_map>

namespace smtk
{
namespace io
{
class Logger;
}
namespace resource
{

class SMTKCORE_EXPORT CopyOptions
{
public:
  /// Control whether copies are omitted, by reference, or by value.
  enum CopyType
  {
    None,    //!< Do not copy data at all; omit it.
    Shallow, //!< Copy data by reference (the source and target will share access).
    Deep     //!< Copy data by value (target and source will initially match; they may diverge).
  };

  /// A type alias for the container holding the UUID translation table.
  using ObjectMapType = std::unordered_map<smtk::common::UUID, smtk::resource::PersistentObject*>;

  CopyOptions();
  CopyOptions(smtk::io::Logger& log);
  virtual ~CopyOptions();

  /// Should the resource location be copied?
  ///
  /// By default, the location of a clone should not be set;
  /// we do not want users to accidentally overwrite the original
  /// resource by saving the new one.
  /// When using clone() to produce an upgraded resource, this
  /// may be set to true.
  bool setCopyLocation(bool copy);
  bool copyLocation() const { return m_copyLocation; }

  /// Should components be copied?
  ///
  /// If true, each component in the \a source should have a matching component
  /// in the \a target resource and an entry in objectMapping() should be added.
  bool setCopyComponents(bool copy);
  bool copyComponents() const { return m_copyComponents; }

  /// Should property data be copied?
  ///
  /// Note that if copyComponents() returns false, only properties corresponding to the resource
  /// are copied. Otherwise, all properties (resource and component) are copied if copyProperties()
  /// returns true.
  bool setCopyProperties(bool copy);
  bool copyProperties() const { return m_copyProperties; }

  /// Should renderable geometry be copied?
  ///
  /// In general, copying geometry should not be required as backend providers should be
  /// able to regenerate the geometry on demand; however, copying this data can save
  /// significant effort since generating renderable data can be slow.
  ///
  /// Note that if copyComponents() returns false, only geometry corresponding to the resource
  /// is copied. Otherwise, all geometry (resource and component) are copied if copyGeometry()
  /// returns true.
  bool setCopyGeometry(bool copy);
  bool copyGeometry() const { return m_copyGeometry; }

  /// Should the template type (and perhaps other template-related data) be copied?
  ///
  /// At a minimum, if this is true, the source and target's templateType() should return
  /// the same value. If templates provide other data, such as an initial set of components
  /// or initialize other resource class-members, that data should be copied when
  /// copyTemplateData() is true.
  bool setCopyTemplateData(bool copy);
  bool copyTemplateData() const { return m_copyTemplateData; }

  /// Copy the version number of the resource template.
  ///
  /// When copyTemplateData() is true, this value determines whether to copy the exact
  /// version number from the source resource or (if false) to use the most recent
  /// version number available.
  bool setCopyTemplateVersion(bool copy);
  bool copyTemplateVersion() const { return m_copyTemplateVersion; }

  /// Should the unit system of the \a source be copied by value, by reference, or omitted?
  bool setCopyUnitSystem(CopyType copy);
  CopyType copyUnitSystem() const { return m_copyUnitSystem; }

  /// Set/get whether or not to copy link data.
  ///
  /// If true (the default), then link data may be excluded by role using
  /// the addLinkRoleToExclude() method.
  /// If false, then linkRolesToExclude() is ignored.
  bool setCopyLinks(bool copy);
  bool copyLinks() const { return m_copyLinks; }

  /// Set/get which link data should **not** be copied.
  ///
  /// By default, all link data will be copied; adding link roles here will omit them
  /// from duplication.
  bool clearLinkRolesToExclude();
  bool addLinkRoleToExclude(smtk::resource::Links::RoleType linkRole);
  bool removeLinkRoleToExclude(smtk::resource::Links::RoleType linkRole);
  const std::set<smtk::resource::Links::RoleType>& linkRolesToExclude() const
  {
    return m_linkRolesToExclude;
  }
  bool shouldExcludeLinksInRole(smtk::resource::Links::RoleType role) const
  {
    return m_linkRolesToExclude.find(role) != m_linkRolesToExclude.end();
  }

  /// Provide a map from source-resource object-IDs to copied-resource object-pointers.
  ///
  /// The Resource::copy() and Resource::resolveCopy() methods should populate and reference
  /// this map, respectively.
  ObjectMapType& objectMapping() { return m_objectMapping; }
  const ObjectMapType& objectMapping() const { return m_objectMapping; }

  /// A convenience to fetch an entry from objectMapping(), casting it to the given type.
  ///
  /// Be aware this method may return a null pointer if (a) there is no
  /// object that corresponds to the input \a sourceId or (b) the object
  /// cannot be cast to \a ObjectType.
  template<typename ObjectType>
  ObjectType* targetObjectFromSourceId(const smtk::common::UUID& sourceId) const
  {
    auto it = m_objectMapping.find(sourceId);
    if (it == m_objectMapping.end())
    {
      return nullptr;
    }
    return dynamic_cast<ObjectType*>(it->second);
  }

  /// Return a set of UUIDs to omit from copying.
  ///
  /// These UUIDs need not necessarily correspond to persistent objects,
  /// although typically this map is populated with component IDs (e.g., by
  /// smtk::resource::Resource::copyProperties() or other methods)
  /// when copyComponents() is false.
  const std::unordered_set<smtk::common::UUID>& omit() const { return m_omit; }
  std::unordered_set<smtk::common::UUID>& omit() { return m_omit; }

  /// Return whether a \a sourceId should be omitted from the copy.
  bool shouldOmitId(const smtk::common::UUID& sourceId) const;

  /// Insert a \a resource's component UUIDs into the omit() set.
  ///
  /// This is used by smtk::resource::Resource::copyProperties() and other
  /// helpers to prevent data related to components by UUID from being
  /// copied.
  void omitComponents(const std::shared_ptr<const Resource>& resource);

  /// Provide a type-container to hold options specific to Resource subclasses.
  ///
  /// For example, you may insert an instance of smtk::attribute::CopyAssignmentOptions
  /// into this container to control how attribute resources are copied.
  const smtk::common::TypeContainer& suboptions() const { return m_suboptions; }
  smtk::common::TypeContainer& suboptions() { return m_suboptions; }

  /// Return a logger to be used to provide feedback on copying.
  smtk::io::Logger& log() const { return *m_log; }

protected:
  bool m_copyTemplateData{ true };
  bool m_copyTemplateVersion{ true };
  CopyType m_copyUnitSystem{ CopyType::Shallow };
  bool m_copyLocation{ false };
  bool m_copyComponents{ true };
  bool m_copyProperties{ true };
  bool m_copyGeometry{ true };
  bool m_copyLinks{ true };
  std::set<smtk::resource::Links::RoleType> m_linkRolesToExclude;

  ObjectMapType m_objectMapping;
  std::unordered_set<smtk::common::UUID> m_omit;
  std::unordered_set<smtk::common::UUID> m_omitComponentsResources;

  smtk::common::TypeContainer m_suboptions;

  mutable smtk::io::Logger* m_log{ nullptr };
  // NOLINTNEXTLINE(modernize-use-default-member-init)
  bool m_deleteLog;
};

} // namespace resource
} // namespace smtk
#endif // smtk_resource_CopyOptions_h
