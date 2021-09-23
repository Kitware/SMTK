//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUIDGenerator.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/LinkInformation.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
class ResourceA;

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

private:
  ComponentA(smtk::resource::ResourcePtr resource)
    : m_resource(resource)
  {
  }

  const smtk::resource::ResourcePtr m_resource;

  int m_value;
  smtk::common::UUID m_id;
};

class ResourceA : public smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>
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

protected:
  ResourceA()
    : smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>()
  {
  }

private:
  std::unordered_set<ComponentA::Ptr> m_components;
};

class ResourceB;

class ComponentB : public smtk::resource::Component
{
  friend class ResourceB;

public:
  smtkTypeMacro(ComponentB);
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
  ComponentB(smtk::resource::ResourcePtr resource)
    : m_resource(resource)
  {
  }

  const smtk::resource::ResourcePtr m_resource;

  int m_value;
  smtk::common::UUID m_id;
};

class ResourceB : public smtk::resource::DerivedFrom<ResourceB, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceB);
  smtkCreateMacro(ResourceB);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  ComponentB::Ptr newComponent()
  {
    ComponentB::Ptr shared(new ComponentB(shared_from_this()));
    shared->setId(smtk::common::UUID::random());
    m_components.insert(shared);
    return std::static_pointer_cast<ComponentB>(shared);
  }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& id) const override
  {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&](const ComponentB::Ptr& c) {
      return c->id() == id;
    });
    return (it != m_components.end() ? *it : smtk::resource::ComponentPtr());
  }

  void remove(const ComponentB::Ptr& component) { m_components.erase(component); }

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
  ResourceB()
    : smtk::resource::DerivedFrom<ResourceB, smtk::resource::Resource>()
  {
  }

private:
  std::unordered_set<ComponentB::Ptr> m_components;
};

template<typename ObjectA, typename ObjectB>
void TestLink(typename ObjectA::Ptr& objectA, typename ObjectB::Ptr& objectB)
{
  smtk::resource::Links::RoleType role1 = 1;

  // Test that the object is not linked to anything.
  smtkTest(
    objectA->links().isLinkedTo(objectB, role1) == false,
    "Object A should not be linked to object B.");

  // Add a link from object A to object B.
  smtk::resource::Links::Key key = objectA->links().addLinkTo(objectB, role1);

  // Test that the returned key is valid.
  smtkTest(
    key.first != smtk::common::UUID::null() && key.second != smtk::common::UUID::null(),
    "Object-to-object link key should not be null.");

  // Test that the API for querying the existence of links is functioning.
  smtkTest(
    objectA->links().isLinkedTo(objectB, role1) == true, "Object A should be linked to object B.");

  // Access the linked object through the link.
  typename ObjectB::Ptr linkedObjectB =
    std::static_pointer_cast<ObjectB>(objectA->links().linkedObject(key));

  // Test that the accessed object is valid.
  smtkTest(linkedObjectB != nullptr, "Link-accessed object should be accessible.");

  // Test that the accessed object is equal to the original object.
  smtkTest(
    linkedObjectB == objectB, "Link-accessed object should be equivalent to original object.");

  // Test the API for accessing all links from an object of a given role type.
  std::set<smtk::resource::PersistentObject::Ptr> linkedTo = objectA->links().linkedTo(role1);

  // There should be only one link.
  smtkTest(linkedTo.size() == 1, "Object A should be linked to one object for this role value.");

  // Cast the pointer to the appropriate type and test for equality to the
  // original object.
  linkedObjectB = std::static_pointer_cast<ObjectB>(*linkedTo.begin());
  smtkTest(
    linkedObjectB == objectB, "Link-accessed object should be equivalent to original object.");

  // Test that queries using a different role value return independent results.
  smtk::resource::Links::RoleType role2 = 2;
  linkedTo = objectA->links().linkedTo(role2);
  smtkTest(
    linkedTo.empty() == true, "Object A should not be linked to any objects for this role value.");

  // Test the API for accessing all links to an object of a given role type.
  std::set<smtk::resource::PersistentObject::Ptr> linkedFrom = objectB->links().linkedFrom(role1);
  smtkTest(
    linkedFrom.size() == 1, "Object B should have one object linked to it for this role value.");

  // Cast the pointer to the appropriate type and test for equality to the
  // original object.
  auto linkedObjectA = std::static_pointer_cast<ObjectA>(*linkedFrom.begin());
  smtkTest(
    linkedObjectA == objectA, "Link-accessed object should be equivalent to original object.");

  // Unlink object B from object A.
  objectA->links().removeLink(key);

  // Test that the link is severed.
  smtkTest(
    objectA->links().isLinkedTo(objectB, role1) == false,
    "Object A should no longer be linked to object B.");
}
} // namespace

int TestResourceLinks(int /*unused*/, char** const /*unused*/)
{
  // Create a ResourceA.
  ResourceA::Ptr resourceA = ResourceA::create();

  smtkTest(resourceA != nullptr, "ResourceA instance should be constructable.");

  // Create a ComponentA.
  ComponentA::Ptr componentA = resourceA->newComponent();

  smtkTest(componentA != nullptr, "ComponentA instance should be constructable.");

  // Make sure the component is working correctly.
  componentA->setValue(3);
  smtkTest(componentA->value() == 3, "ComponentA instance value should be settable.");

  // Create a ResourceB.
  ResourceB::Ptr resourceB = ResourceB::create();

  smtkTest(resourceB != nullptr, "ResourceB instance should be constructable.");

  // Create a ComponentB.
  ComponentB::Ptr componentB = resourceB->newComponent();

  smtkTest(componentB != nullptr, "ComponentB instance should be constructable.");

  // Make sure the component is working correctly.
  componentB->setValue(4);
  smtkTest(componentB->value() == 4, "ComponentB instance value should be settable.");

  // Create a resource manager (needed for reverse lookup)
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  // Register ResourceA and ResourceB
  resourceManager->registerResource<ResourceA>();
  resourceManager->registerResource<ResourceB>();

  // Add resourceA and resourceB
  resourceManager->add(resourceA);
  resourceManager->add(resourceB);

  TestLink<ResourceA, ResourceB>(resourceA, resourceB);
  TestLink<ResourceA, ComponentB>(resourceA, componentB);
  TestLink<ComponentA, ResourceB>(componentA, resourceB);
  TestLink<ComponentA, ComponentB>(componentA, componentB);

  // Test the ability to remove all links between resources
  {
    smtk::resource::Links::RoleType role1 = 1;

    // Add a link from component A to component B.
    smtk::resource::Links::Key key = componentA->links().addLinkTo(componentB, role1);
    (void)key;

    smtkTest(
      resourceA->links().removeAllLinksTo(resourceB),
      "Could not remove all links between resources");

    smtkTest(resourceA != nullptr, "ResourceA instance should be constructable.");

    // Test that the component link is severed.
    smtkTest(
      componentA->links().isLinkedTo(componentB, role1) == false,
      "Component A should no longer be linked to Component B.");
  }

  {
    smtk::resource::Links::RoleType role1 = 1;

    // Add a link from component A to component B.
    smtk::resource::Links::Key key = componentA->links().addLinkTo(componentB, role1);

    {
      // Access information about the linked object.
      smtk::resource::LinkInformation information =
        componentA->links().linkedObjectInformation(key);

      smtkTest(
        information.type == smtk::resource::LinkInformation::Type::Component,
        "Incorrectly identified link type.");
      smtkTest(
        information.status == smtk::resource::LinkInformation::Status::Valid,
        "Incorrectly identified link status.");
      smtkTest(information.id == componentB->id(), "Incorrectly identified linked object id.");
      smtkTest(information.role == role1, "Incorrectly identified linked object role.");
    }

    // Remove component B from resource B.
    resourceB->remove(componentB);

    // Test that component A is still linked to component B.
    smtkTest(
      componentA->links().isLinkedTo(componentB, role1) == true,
      "Component A should be linked to component B even after component B \n"
      "is removed from Resource B.");

    // Access the linked object through the link.
    typename ComponentB::Ptr linkedObjectB =
      std::static_pointer_cast<ComponentB>(componentA->links().linkedObject(key));

    // Test that the accessed object is invalid.
    smtkTest(linkedObjectB == nullptr, "Link-accessed component should no longer be accessible.");

    // Test that the accessed object id is correct.
    smtkTest(
      componentB->id() == componentA->links().linkedObjectId(key),
      "Link-accessed id should be accessible.");

    {
      // Access information about the linked object.
      smtk::resource::LinkInformation information =
        componentA->links().linkedObjectInformation(key);

      smtkTest(
        information.type == smtk::resource::LinkInformation::Type::Component,
        "Incorrectly identified link type.");
      smtkTest(
        information.status == smtk::resource::LinkInformation::Status::Invalid,
        "Incorrectly identified link status.");
      smtkTest(information.id == componentB->id(), "Incorrectly identified linked object id.");
      smtkTest(information.role == role1, "Incorrectly identified linked object role.");
    }

    resourceManager->remove(resourceB);
    smtk::common::UUID componentB_id = componentB->id();
    componentB.reset();
    resourceB.reset();

    {
      // Access information about the linked object.
      smtk::resource::LinkInformation information =
        componentA->links().linkedObjectInformation(key);

      smtkTest(
        information.type == smtk::resource::LinkInformation::Type::Component,
        "Incorrectly identified link type.");
      smtkTest(
        information.status == smtk::resource::LinkInformation::Status::Unknown,
        "Incorrectly identified link status.");
      smtkTest(information.id == componentB_id, "Incorrectly identified linked object id.");
      smtkTest(information.role == role1, "Incorrectly identified linked object role.");
    }
  }

  return 0;
}
