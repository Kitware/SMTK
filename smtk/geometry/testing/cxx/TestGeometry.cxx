//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/geometry/Backend.h"
#include "smtk/geometry/Cache.h"
#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/DerivedFrom.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <memory>
#include <sstream>
#include <string>

namespace detail
{
class ResourceA;

// Example of a custom "geometry" format for a backend.
// We cannot use smtk::geometry::GeometryForBackend<int>
// because integers are not automatically initialized.
// Here, we just wrap an integer so that Format is
// initialized by default and convertible to a bool.
// Usually, you will GeometryForBackend a shared or
// smart pointer to some more complex structure.
struct Format
{
  int m_data = 0;
  Format() = default;
  Format(int data)
    : m_data(data)
  {
  }
  Format(const Format& other) = default;
  operator bool() const { return m_data <= 0; }
  operator int() const { return m_data <= 0 ? -1 : m_data; }
};

class Backend1 : public smtk::geometry::Backend
{
public:
  Backend1() = default;
  ~Backend1() override = default;

  std::string name() const override { return "Backend1"; }
};

class Backend2 : public smtk::geometry::Backend
{
public:
  Backend2() = default;
  ~Backend2() override = default;

  std::string name() const override { return "Backend2"; }

  template<typename Geometry>
  int geometry(const Geometry& p, const smtk::resource::PersistentObject::Ptr& obj)
  {
    int val;
    try
    {
      const auto& provider = dynamic_cast<const smtk::geometry::GeometryForBackend<Format>&>(*p);
      val = provider.data(obj);
    }
    catch (std::bad_cast&)
    {
      val = -1;
    }
    return val;
  }
};

class ComponentA : public smtk::resource::Component
{
  friend class ResourceA;

public:
  smtkTypeMacro(ComponentA);
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

  std::string name() const override
  {
    std::ostringstream result;
    result << "Component " << m_value << " (" << m_id << ")";
    return result.str();
  }

private:
  ComponentA(smtk::resource::ResourcePtr resource)
    : m_resource(resource)
  {
  }

  const smtk::resource::ResourcePtr m_resource;

  int m_value;
  smtk::common::UUID m_id;
};

class ResourceA : public smtk::resource::DerivedFrom<ResourceA, smtk::geometry::Resource>
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  ComponentA::Ptr newComponent()
  {
    ComponentA::Ptr shared(new ComponentA(shared_from_this()));
    shared->setId(smtk::common::UUID::random());
    m_components.insert(shared);
    return std::static_pointer_cast<ComponentA>(shared);
  }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& id) const override
  {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&](const ComponentA::Ptr& c) {
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

  bool erase(const ComponentA::Ptr& comp) { return m_components.erase(comp) > 0; }

  std::size_t size() const { return m_components.size(); }

protected:
  ResourceA() = default;

private:
  std::unordered_set<ComponentA::Ptr> m_components;
};

class Geometry2 : public smtk::geometry::Cache<smtk::geometry::GeometryForBackend<Format>>
{
public:
  Geometry2(const ResourceA::Ptr& parent)
    : m_parent(parent)
  {
  }
  ~Geometry2() override = default;

  const smtk::geometry::Backend& backend() const override
  {
    static Backend2 data;
    return data;
  }

  smtk::geometry::Resource::Ptr resource() const override { return m_parent.lock(); }

  void queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
    const override
  {
    auto comp = std::dynamic_pointer_cast<ComponentA>(obj);
    if (comp && comp->value() >= 0)
    {
      entry.m_geometry = comp->value();
      entry.m_generation = Initial;
    }
    else
    {
      entry.m_generation = Invalid;
    }
  }

  void geometricBounds(const Format& value, BoundingBox& bds) const override
  {
    bds[0] = bds[2] = bds[4] = +0.0;
    bds[1] = bds[3] = bds[5] = static_cast<double>((int)value);
  }

  // Ensure every component with renderable geometry has a cache entry.
  void update() const override
  {
    std::cout << "  Updating geometry before iteration\n";
    auto rsrc = m_parent.lock();
    if (rsrc)
    {
      smtk::resource::Component::Visitor visitor =
        [this](const smtk::resource::Component::Ptr& comp) {
          auto compA = std::dynamic_pointer_cast<ComponentA>(comp);
          if (compA && compA->value() >= 0)
          {
            m_cache[compA->id()] = CacheEntry{ Initial, compA->value() };
          }
          else
          {
            m_cache.erase(compA->id());
          }
        };
      rsrc->visit(visitor);
      // Also erase the cache entry for the resource itself (since
      // we know it does not have valid geometry).
      m_cache.erase(rsrc->id());
    }
  }

  ResourceA::WeakPtr m_parent;
};

class RegisterResourceABackend2 : public smtk::geometry::Supplier<RegisterResourceABackend2>
{
public:
  bool valid(const smtk::geometry::Specification& in) const override
  {
    Backend2 backend;
    return std::get<1>(in).index() == backend.index();
  }

  smtk::geometry::GeometryPtr operator()(const smtk::geometry::Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<ResourceA>(std::get<0>(in));
    if (rsrc)
    {
      auto* provider = new Geometry2(rsrc);
      return smtk::geometry::GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a test resource.");
    return nullptr;
  }
};

static bool registeredA2 = RegisterResourceABackend2::registerClass();

} // namespace detail

using namespace detail;

int TestGeometry(int /*unused*/, char** const /*unused*/)
{
  // Create managers
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  auto geometryManager = smtk::geometry::Manager::create();

  // Cause creation of resources to create geometry objects for all registered backends
  geometryManager->registerResourceManager(resourceManager);

  // Register ResourceA
  resourceManager->registerResource<ResourceA>();

  // Create a new ResourceA type
  auto resourceA = resourceManager->create<ResourceA>();
  resourceA->setName("Resource A");

  auto comp42 = resourceA->newComponent();
  auto comp43 = resourceA->newComponent();
  auto compM1 = resourceA->newComponent();
  auto compM2 = resourceA->newComponent();
  comp42->setValue(42);
  comp43->setValue(43);
  compM1->setValue(-1);
  compM2->setValue(-2);

  int count = 0;
  auto visitor = [&count](const smtk::geometry::Backend& visited) {
    std::cout << "  Found " << visited.name() << "\n";
    ++count;
  };
  geometryManager->visitBackends(visitor);
  smtkTest(count == 0, "Expected 0 backends before registration.");

  count = 0;
  std::cout << "Registering backends\n";
  geometryManager->registerBackend<detail::Backend1>();
  geometryManager->registerBackend<detail::Backend2>();
  geometryManager->visitBackends(visitor);
  smtkTest(count == 2, "Expected 2 backends after registration.");

  count = 0;
  std::cout << "Unregistering 1 backend\n";
  geometryManager->unregisterBackend<detail::Backend1>();
  geometryManager->visitBackends(visitor);
  smtkTest(count == 1, "Expected 1 backend after unregistration.");

  auto& geomA1 = resourceA->geometry(detail::Backend1{});
  smtkTest(geomA1 == nullptr, "Expected no geometry for Backend1");

  auto& geomA2 = resourceA->geometry(detail::Backend2{});
  smtkTest(geomA2 != nullptr, "Expected a geometry for Backend2");

  // Test that the caching geometry provider calls Geometry2::update()
  std::cout << "Visiting renderables\n";
  smtk::geometry::Geometry::BoundingBox bbox;
  count = 0;
  geomA2->visit([&geomA2, &bbox, &count](
                  const smtk::resource::PersistentObject::Ptr& obj,
                  smtk::geometry::Geometry::GenerationNumber) {
    if (obj)
    {
      // Here is an example of how to fetch the "actual"
      // data from a geometry instance using a specific backend:
      detail::Backend2 renderer;
      int geom = renderer.geometry(geomA2, obj);

      geomA2->bounds(obj, bbox);
      std::cout << "    object \"" << obj->name() << "\" geometry " << geom << "\n";

      smtkTest(geom > 0, "Did not expect to visit object with invalid geometry.");
      smtkTest(bbox[1] >= bbox[0], "Expected a valid bounding box.");
      ++count;
    }
    return false;
  });
  smtkTest(count == 2, "Expected to visit 2 components with valid \"geometry\".");

  auto generation = geomA2->generationNumber(compM1);
  smtkTest(
    generation == smtk::geometry::Geometry::Invalid,
    "Expected invalid generation number for component with negative value.");

  smtk::geometry::Geometry::BoundingBox bds;
  geomA2->bounds(compM1, bds);
  smtkTest(
    bds[1] < bds[0] && bds[3] < bds[2] && bds[5] < bds[4],
    "Expected invalid bounds for component with no \"geometry\".");

  return 0;
}
