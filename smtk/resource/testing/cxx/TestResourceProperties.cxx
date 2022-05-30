//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/Properties.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/common/UUID.h"
#include "smtk/common/json/jsonUUID.h"
#include "smtk/common/testing/cxx/helpers.h"

namespace
{
const double double_epsilon = 1.e-10;

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
} // namespace

int TestResourceProperties(int /*unused*/, char** const /*unused*/)
{
  {
    // Create a Resource.
    Resource::Ptr resource = Resource::create();

    smtkTest(resource != nullptr, "Resource instance should be constructable.");

    // Create a Component.
    Component::Ptr component = resource->newComponent();

    // Test assignment.
    resource->properties().get<long>()["foo"] = 2;
    test(resource->properties().get<long>()["foo"] == 2, "Value incorrectly assigned");

    // Test existence.
    test(resource->properties().contains<long>("foo"), "Value incorrectly assigned");
    test(!component->properties().contains<long>("foo"), "Value incorrectly assigned");

    try
    {
      long i = component->properties().get<long>().at("foo");
      test(false, "An \"out of range\" exception should have been thrown.");
      (void)i;
    }
    catch (const std::out_of_range&)
    {
    }

    component->properties().emplace<double>("foo", 2.3);
    test(component->properties().contains<double>("foo"), "Value incorrectly assigned");
    test(
      fabs(component->properties().at<double>("foo") - 2.3) < double_epsilon,
      "Value incorrectly emplaced");

    test(
      resource->properties().get<std::vector<std::string>>()["bar"].empty(),
      "Array-style access should implicitly create property type");

    resource->properties().erase<double>("bar"); // Try to erase a value not present.
    resource->properties().erase<double>("foo");
    test(
      !resource->properties().contains<double>("foo"), "Previously erased value still accessible.");
  }

  std::cout << "destructor works" << std::endl;

  return 0;
}
