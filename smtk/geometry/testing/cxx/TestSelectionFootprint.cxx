//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/CellEntity.h"
#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Shell.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"

#include "smtk/geometry/Backend.h"
#include "smtk/geometry/Cache.h"
#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"
#include "smtk/geometry/queries/SelectionFootprint.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/plugin/Registry.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <memory>
#include <sstream>
#include <string>

namespace
{

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

class Backend : public smtk::geometry::Backend
{
public:
  Backend() = default;
  ~Backend() override = default;

  [[nodiscard]] std::string name() const override { return "Backend"; }
};

class Geometry : public smtk::geometry::Cache<smtk::geometry::GeometryForBackend<Format>>
{
public:
  smtkTypeMacro(Geometry);
  Geometry(const smtk::model::Resource::Ptr& parent)
    : m_parent(parent)
  {
  }
  ~Geometry() override = default;

  const smtk::geometry::Backend& backend() const override
  {
    static Backend data;
    return data;
  }

  smtk::geometry::Resource::Ptr resource() const override { return m_parent.lock(); }

  void queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
    const override
  {
    auto comp = std::dynamic_pointer_cast<smtk::model::Entity>(obj);
    if (comp && comp->isCellEntity())
    {
      entry.m_geometry = 42;
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
    auto rsrc = m_parent.lock();
    if (rsrc)
    {
      // Populate cells with valid geometry; eliminate all others.
      smtk::resource::Component::Visitor visitor =
        [this](const smtk::resource::Component::Ptr& comp) {
          auto ent = std::dynamic_pointer_cast<smtk::model::Entity>(comp);
          if (ent && ent->isCellEntity())
          {
            m_cache[ent->id()] = CacheEntry{ Initial, 1 };
          }
          else
          {
            m_cache.erase(ent->id());
          }
        };
      rsrc->visit(visitor);
      // Also erase the cache entry for the resource itself (since
      // we know it does not have valid geometry).
      m_cache.erase(rsrc->id());
    }
  }

  smtk::model::Resource::WeakPtr m_parent;
};

class RegisterBackend : public smtk::geometry::Supplier<RegisterBackend>
{
public:
  [[nodiscard]] bool valid(const Specification& in) const override
  {
    Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  std::unique_ptr<smtk::geometry::Geometry> operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::model::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto* provider = new Geometry(rsrc);
      return std::unique_ptr<smtk::geometry::Geometry>(provider);
    }
    throw std::invalid_argument("Not a model resource.");
    return nullptr;
  }
};

template<typename Footprint>
void dump(const Footprint& footprint, const std::string& msg)
{
  std::cout << msg << "\n";
  bool any = false;
  for (const auto& entry : footprint)
  {
    any = true;
    std::cout << "  " << entry->name() << "\n";
  }
  if (!any)
  {
    std::cout << "  empty\n";
  }
}

} // namespace

int TestSelectionFootprint(int /*unused*/, char** const /*unused*/)
{
  // Create managers
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();
  auto geometryManager = smtk::geometry::Manager::create();

  // Cause creation of resources to create geometry objects for all registered backends
  geometryManager->registerResourceManager(resourceManager);
  geometryManager->registerBackend<Backend>();
  RegisterBackend::registerClass();

  auto modelRegistry =
    smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager, operationManager);

  auto attributeRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager, operationManager);

  // Create a new model resource type
  auto modelRsrc = resourceManager->create<smtk::model::Resource>();
  modelRsrc->setName("Resource A");

  smtk::operation::MarkGeometry marker(modelRsrc);
  auto model = modelRsrc->addModel(3, 3, "test model");
  auto group = modelRsrc->addGroup(0, "test group");
  auto face0 = modelRsrc->addFace();
  auto face1 = modelRsrc->addFace();
  auto face2 = modelRsrc->addFace();
  auto edge0 = modelRsrc->addEdge();
  auto edge1 = modelRsrc->addEdge();
  auto shell = modelRsrc->addShell(); // Add something purposefully empty.
  face0.setName("face0");
  face1.setName("face1");
  face2.setName("face2");
  edge0.setName("edge0");
  edge1.setName("edge1");
  shell.setName("shell");
  marker.markModified(face0.component());
  marker.markModified(face1.component());
  marker.markModified(face2.component());
  marker.markModified(edge0.component());
  marker.markModified(edge1.component());
  marker.markModified(shell.component());
  model.addCell(face0);
  model.addCell(face1);
  model.addCell(face2);
  model.addGroup(group);
  group.addEntity(face0);
  group.addEntity(edge1);
  face0.addRawRelation(edge0);
  face0.addRawRelation(edge1);
  face1.addRawRelation(edge1);
  edge0.findOrAddRawRelation(face0);
  edge0.findOrAddRawRelation(face1);
  edge1.findOrAddRawRelation(face1);

  auto attrRsrc = resourceManager->create<smtk::attribute::Resource>();
  attrRsrc->setName("Resource B");
  auto def0 = attrRsrc->createDefinition("test def composite assoc");
  auto def1 = attrRsrc->createDefinition("test def cell assoc");
  auto def2 = attrRsrc->createDefinition("test def no assoc");
  auto asr0 = def0->createLocalAssociationRule();
  asr0->setAcceptsEntries("smtk::model::Resource", "any", true);
  asr0->setIsExtensible(true);
  auto asr1 = def1->createLocalAssociationRule();
  asr1->setAcceptsEntries("smtk::model::Resource", "cell|anydim", true);
  asr1->setIsExtensible(true);
  // def2 will not allow association by default.
  auto att0 = attrRsrc->createAttribute("test attribute model-assoc", def0);
  auto att1 = attrRsrc->createAttribute("test attribute group-assoc", def0);
  auto att2 = attrRsrc->createAttribute("test attribute cell-assoc", def1);
  auto att3 = attrRsrc->createAttribute("test attribute no-assoc", def2);
  smtkTest(att0->associate(model.component()), "Could not associate model to att0.");
  smtkTest(att1->associate(group.component()), "Could not associate group to att1.");
  smtkTest(att2->associate(face0.component()), "Could not associate face0 to att2.");
  smtkTest(att2->associate(face1.component()), "Could not associate face1 to att2.");
  smtkTest(att2->associate(face2.component()), "Could not associate face2 to att2.");

  dynamic_cast<Geometry*>(modelRsrc->geometry(Backend()).get())->update();
  auto& asf = attrRsrc->queries().get<smtk::geometry::SelectionFootprint>();
  auto& msf = modelRsrc->queries().get<smtk::geometry::SelectionFootprint>();

  std::unordered_set<smtk::resource::PersistentObject*> modelFootprint;
  msf(*model.component(), modelFootprint, Backend());
  dump(modelFootprint, "footprint(model)");
  smtkTest(modelFootprint.size() == 5, "Expected |footprint(model)| == 5.");

  std::unordered_set<smtk::resource::PersistentObject*> groupFootprint;
  msf(*group.component(), groupFootprint, Backend());
  dump(groupFootprint, "footprint(group)");
  smtkTest(groupFootprint.size() == 2, "Expected |footprint(group)| == 2.");

  std::unordered_set<smtk::resource::PersistentObject*> face0Footprint;
  msf(*face0.component(), face0Footprint, Backend());
  dump(face0Footprint, "footprint(face0)");
  smtkTest(
    face0Footprint.size() == 1 && face0Footprint.count(face0.component().get()) == 1,
    "Expected |footprint(face0)| == 1.");

  std::unordered_set<smtk::resource::PersistentObject*> shellFootprint;
  msf(*shell.component(), shellFootprint, Backend());
  dump(shellFootprint, "footprint(shell)");
  smtkTest(shellFootprint.empty(), "Expected |footprint(shell)| == 0.");

  std::unordered_set<smtk::resource::PersistentObject*> att0Footprint;
  asf(*att0, att0Footprint, Backend());
  dump(att0Footprint, "footprint(model-assoc attribute)");
  smtkTest(
    att0Footprint == modelFootprint,
    "Expected footprint(model-assoc attribute) == footprint(model).");

  std::unordered_set<smtk::resource::PersistentObject*> att1Footprint;
  asf(*att1, att1Footprint, Backend());
  dump(att1Footprint, "footprint(group-assoc attribute)");
  smtkTest(
    att1Footprint == groupFootprint,
    "Expected footprint(group-assoc attribute) == footprint(group).");

  std::unordered_set<smtk::resource::PersistentObject*> att2Footprint;
  asf(*att2, att2Footprint, Backend());
  dump(att2Footprint, "footprint(face-assoc attribute)");
  smtkTest(att2Footprint.size() == 3, "Expected |footprint(face-assoc attribute)| == 3.");

  std::unordered_set<smtk::resource::PersistentObject*> att3Footprint;
  asf(*att3, att3Footprint, Backend());
  dump(att3Footprint, "footprint(no-assoc attribute)");
  smtkTest(att3Footprint.empty(), "Expected |footprint(no-assoc attribute)| = 0.");

  return 0;
}
