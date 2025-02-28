//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/query/Cache.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/UUID.h"
#include "smtk/common/testing/cxx/helpers.h"

namespace
{
class Resource;

class Component : public smtk::resource::Component
{
  friend class Resource;

public:
  smtkTypeMacro(Component);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  const smtk::resource::ResourcePtr resource() const override { return this->m_resource; }

  int value() const { return m_value; }
  void setValue(int v) { m_value = v; }

  const smtk::common::UUID& id() const override { return m_id; }
  bool setId(const smtk::common::UUID& id) override
  {
    m_id = id;
    return true;
  }

private:
  Component(smtk::resource::ResourcePtr resource)
    : m_resource(resource)
  {
  }

  const smtk::resource::ResourcePtr m_resource;

  int m_value;
  smtk::common::UUID m_id;
};

class Resource : public smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(Resource);
  smtkCreateMacro(Resource);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  Component::Ptr newComponent()
  {
    Component::Ptr shared(new Component(shared_from_this()));
    shared->setId(smtk::common::UUID::random());
    m_components.insert(shared);
    return std::static_pointer_cast<Component>(shared);
  }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& id) const override
  {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&](const Component::Ptr& c) {
      return c->id() == id;
    });
    return (it != m_components.end() ? *it : smtk::resource::ComponentPtr());
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& visitor) const override
  {
    std::for_each(m_components.begin(), m_components.end(), visitor);
  }

protected:
  Resource() = default;

private:
  std::unordered_set<Component::Ptr> m_components;
};

struct MyCache : smtk::resource::query::Cache
{
  std::unordered_map<smtk::common::UUID, int> m_cachedValues;
};

enum QueryResult
{
  CachedValue = 0,
  NewValue = 1,
};

class Query : public smtk::resource::query::Query
{
public:
  [[nodiscard]] int performQuery(const smtk::resource::ComponentPtr& component) const
  {
    // Access the query artifact from the resource
    smtk::resource::Resource::Ptr resource = component->resource();
    MyCache& queryCache = resource->queries().cache<MyCache>();

    // Perform a calculation using the query artifact
    auto it = queryCache.m_cachedValues.find(component->id());
    if (it != queryCache.m_cachedValues.end())
    {
      return it->second;
    }
    queryCache.m_cachedValues[component->id()] = QueryResult::CachedValue;

    return QueryResult::NewValue;
  }
};
} // namespace

int TestResourceQueries(int /*unused*/, char** const /*unused*/)
{
  {
    // Create a Resource.
    Resource::Ptr resource = Resource::create();

    smtkTest(resource != nullptr, "Resource instance should be constructable.");

    // Create a Component.
    Component::Ptr component = resource->newComponent();

    {
      Query query;
      test(
        query.performQuery(component) == QueryResult::NewValue,
        "First query return unexpected value");
      test(
        query.performQuery(component) == QueryResult::CachedValue,
        "Second query return unexpected value");
    }

    test(
      Query().performQuery(component) == QueryResult::CachedValue,
      "Third query return unexpected value");
  }

  return 0;
}
