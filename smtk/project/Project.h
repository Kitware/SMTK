//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Project_h
#define smtk_project_Project_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/resource/DerivedFrom.h"

#include "smtk/task/Manager.h"

#include "smtk/project/OperationFactory.h"
#include "smtk/project/ResourceContainer.h"

#include <boost/type_index.hpp>

#include <memory> // for shared_ptr

namespace smtk
{
namespace project
{

class Manager;

template<typename Self, typename Parent>
using DerivedFrom = smtk::resource::DerivedFrom<Self, Parent>;

/// A Project represents an encapsulation of a subset of SMTK's Resources and
/// Operations for the purpose of accomplishing a targeted set of tasks. It
/// contains Resources and a list of Operations that are pertinent to the
/// Project. As a descendent of Resource, it also contains links, properties, and
/// Query functionality.
class SMTKCORE_EXPORT Project
  : public smtk::resource::DerivedFrom<Project, smtk::resource::Resource>
{
  friend class Manager;

public:
  smtkTypedefs(smtk::project::Project);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  static constexpr const char* const type_name = "smtk::project::Project";
  std::string typeName() const override { return (m_typeName.empty() ? type_name : m_typeName); }
  smtk::string::Token typeToken() const override { return this->typeName(); }
  std::vector<smtk::string::Token> classHierarchy() const override
  {
    static std::vector<smtk::string::Token> baseTypes;
    if (baseTypes.empty())
    {
      smtk::common::typeHierarchy<smtk::project::Project>(baseTypes);
    }
    std::vector<smtk::string::Token> result;
    result.emplace_back(this->typeName());
    result.insert(result.end(), baseTypes.begin(), baseTypes.end());
    return result;
  }
  bool matchesType(smtk::string::Token candidate) const override
  {
    static std::unordered_set<smtk::string::Token> baseTypes;
    if (baseTypes.empty())
    {
      smtk::common::typeHierarchy<smtk::project::Project>(baseTypes);
    }
    return candidate == this->typeToken() || baseTypes.find(candidate) != baseTypes.end();
  }

  static std::shared_ptr<smtk::project::Project> create(const std::string& typeName = "");

  /// A hash value uniquely representing the project type.
  typedef std::size_t Index;

  ~Project() override;

  Index index() const override
  {
    return (
      m_typeName.empty() ? std::type_index(typeid(*this)).hash_code()
                         : std::hash<std::string>{}(m_typeName));
  }

  /// Access the Project's Resources.
  const ResourceContainer& resources() const { return m_resources; }
  ResourceContainer& resources() { return m_resources; }

  /// Access the Project's Operations.
  const OperationFactory& operations() const { return m_operations; }
  OperationFactory& operations() { return m_operations; }

  /// Access the Project's version string.
  const std::string& version() const { return m_version; }
  void setVersion(const std::string& version) { m_version = version; }

  /// Return a functor that evaluates whether a component matches the \a query string.
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& query) const override;

  /// Visit components this project owns.
  void visit(smtk::resource::Component::Visitor&) const override;

  /// Return a project component pointer given its UUID.
  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override;

  const smtk::project::Manager* manager() const { return m_manager; }

  /// Return this project's task manager.
  const smtk::task::Manager& taskManager() const { return *m_taskManager; }
  smtk::task::Manager& taskManager() { return *m_taskManager; }

  bool clean() const override;

protected:
  Project(const std::string& typeName = "");

private:
  ResourceContainer m_resources;
  OperationFactory m_operations;
  std::string m_typeName;
  std::string m_version;
  smtk::project::Manager* m_manager{ nullptr };
  std::shared_ptr<smtk::task::Manager> m_taskManager;
};
} // namespace project
} // namespace smtk

#endif
