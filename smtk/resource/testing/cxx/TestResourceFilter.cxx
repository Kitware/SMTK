//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/TypeName.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/PersistentObject.h"

#include "smtk/resource/filter/Filter.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <string>

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

int TestResourceFilter(int /*unused*/, char** const /*unused*/)
{
  Resource::Ptr resource = Resource::create();

  smtkTest(resource != nullptr, "Resource instance should be constructable.");

  Component::Ptr component1 = resource->newComponent();

  component1->properties().emplace<long>("foo", 2);

  Component::Ptr component2 = resource->newComponent();
  component2->properties().emplace<std::string>("foo", "bar");

  Component::Ptr component3 = resource->newComponent();
  component3->properties().emplace<double>("foo", 3.14159);

  std::array<Component*, 3> components = { component1.get(), component2.get(), component3.get() };

  auto queryOp1 = resource->queryOperation("[ integer { 'foo' }]");
  auto queryOp2 = resource->queryOperation("[ integer { 'foo' = 2 }]");
  auto queryOp3 = resource->queryOperation("[ integer { 'foo' = 3 }]");
  auto queryOp4 = resource->queryOperation("[ string { /f.o/ }]");
  auto queryOp5 = resource->queryOperation("[ string { /f.o/ = /b.r/ } ]");
  auto queryOp6 = resource->queryOperation("[ string { /f.o/ = /c.r/ } ]");
  auto queryOp7 = resource->queryOperation("[ floating-point { /f.o/ }]");
  auto queryOp8 = resource->queryOperation("[ floating-point { /f.o/ = 3.14159 } ]");
  auto queryOp9 = resource->queryOperation("[ floating-point { /f.o/ = 2.71828 } ]");

  std::array<decltype(queryOp1)*, 9> queryOps = { &queryOp1, &queryOp2, &queryOp3,
                                                  &queryOp4, &queryOp5, &queryOp6,
                                                  &queryOp7, &queryOp8, &queryOp9 };

  std::array<std::array<bool, 3>, 9> expected = { { { true, false, false },
                                                    { true, false, false },
                                                    { false, false, false },
                                                    { false, true, false },
                                                    { false, true, false },
                                                    { false, false, false },
                                                    { false, false, true },
                                                    { false, false, true },
                                                    { false, false, false } } };

  for (std::size_t i = 0; i < 9; ++i)
  {
    for (std::size_t j = 0; j < 3; ++j)
    {
      test(
        (*queryOps[i])(*components[j]) == expected[i][j],
        "Filter operation " + std::to_string(i) + ", " + std::to_string(j) +
          " returned unexpected result");
    }
  }

  Component::Ptr component4 = resource->newComponent();
  component4->properties().emplace<std::vector<long>>("foo", { 0, 1, 2 });

  Component::Ptr component5 = resource->newComponent();
  component5->properties().emplace<std::vector<std::string>>("foo", { "bar", "baz", "bat" });

  Component::Ptr component6 = resource->newComponent();
  component6->properties().emplace<std::vector<double>>("foo", { 0.1, 2.3, 4.5 });

  std::array<Component*, 3> components2 = { component4.get(), component5.get(), component6.get() };

  auto queryOp10 = resource->queryOperation("[ vector<int> { 'foo' }]");
  auto queryOp11 = resource->queryOperation("[ vector<int> { 'foo' = (0, 1, 2)}]");
  auto queryOp12 = resource->queryOperation("[ vector<int> { 'foo' = (0, 1, 3)}]");
  auto queryOp13 = resource->queryOperation("[ vector<string> { 'foo' }]");
  auto queryOp14 = resource->queryOperation("[ vector<string> { 'foo' = ('bar', 'baz', 'bat')}]");
  auto queryOp15 = resource->queryOperation("[ vector<string> { 'foo' = ('foo', 'foo', 'foo')}]");
  auto queryOp16 = resource->queryOperation("[ vector<floating-point> { 'foo'}]");
  auto queryOp17 = resource->queryOperation("[ vector<floating-point> { 'foo' = (0.1, 2.3, 4.5)}]");
  auto queryOp18 = resource->queryOperation("[ vector<floating-point> { 'foo' = (0.2, 4.6, 8.1)}]");

  std::array<decltype(queryOp1)*, 9> queryOps2 = { &queryOp10, &queryOp11, &queryOp12,
                                                   &queryOp13, &queryOp14, &queryOp15,
                                                   &queryOp16, &queryOp17, &queryOp18 };

  std::array<std::array<bool, 3>, 9> expected2 = { { { true, false, false },
                                                     { true, false, false },
                                                     { false, false, false },
                                                     { false, true, false },
                                                     { false, true, false },
                                                     { false, false, false },
                                                     { false, false, true },
                                                     { false, false, true },
                                                     { false, false, false } } };

  for (std::size_t i = 0; i < 9; ++i)
  {
    for (std::size_t j = 0; j < 3; ++j)
    {
      test(
        (*queryOps2[i])(*components2[j]) == expected2[i][j],
        "Filter operation " + std::to_string(i) + ", " + std::to_string(j) +
          " returned unexpected result");
    }
  }

  return 0;
}
