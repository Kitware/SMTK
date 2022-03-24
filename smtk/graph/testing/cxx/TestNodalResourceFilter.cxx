//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

namespace test_nodal_resource_filter
{
class NodeA : public smtk::graph::Component
{
public:
  smtkTypenameMacro(NodeA);
  template<typename... Args>
  NodeA(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

class NodeB : public smtk::graph::Component
{
public:
  smtkTypenameMacro(NodeB);
  template<typename... Args>
  NodeB(Args&&... args)
    : smtk::graph::Component::Component(std::forward<Args>(args)...)
  {
  }
};

struct BasicTraits
{
  typedef std::tuple<test_nodal_resource_filter::NodeA, test_nodal_resource_filter::NodeB>
    NodeTypes;
  typedef std::tuple<> ArcTypes;
};
} // namespace test_nodal_resource_filter

int TestNodalResourceFilter(int, char*[])
{
  auto resource = smtk::graph::Resource<test_nodal_resource_filter::BasicTraits>::create();

  std::cout << resource->typeName() << std::endl;

  auto nodeA = resource->create<test_nodal_resource_filter::NodeA>();
  nodeA->properties().emplace<long>("foo", 2);
  nodeA->properties().emplace<std::string>("foo", "bar");
  nodeA->properties().emplace<double>("foo", 3.14159);

  auto nodeB = resource->create<test_nodal_resource_filter::NodeB>();
  nodeB->properties().emplace<long>("foo", 2);
  nodeB->properties().emplace<std::string>("foo", "bar");
  nodeB->properties().emplace<double>("foo", 3.14159);

  auto queryOp1A = resource->queryOperation("'NodeA' [ integer { 'foo' }]");
  auto queryOp2A = resource->queryOperation("/N.deA/ [ integer { 'foo' = 2 }]");
  auto queryOp3A = resource->queryOperation("'NodeA' [ integer { 'foo' = 3 }]");
  auto queryOp4A = resource->queryOperation("'NodeA' [ string { /f.o/ }]");
  auto queryOp5A = resource->queryOperation("/N.deA/ [ string { /f.o/ = /b.r/ } ]");
  auto queryOp6A = resource->queryOperation("'NodeA' [ string { /f.o/ = /c.r/ } ]");
  auto queryOp7A = resource->queryOperation("'NodeA' [ floating-point { /f.o/ }]");
  auto queryOp8A = resource->queryOperation("/N.deA/ [ floating-point { /f.o/ = 3.14159 } ]");
  auto queryOp9A = resource->queryOperation("'NodeA' [ floating-point { /f.o/ = 2.71828 } ]");

  auto queryOp1B = resource->queryOperation("'NodeB' [ integer { 'foo' }]");
  auto queryOp2B = resource->queryOperation("'NodeB' [ integer { 'foo' = 2 }]");
  auto queryOp3B = resource->queryOperation("'NodeB' [ integer { 'foo' = 3 }]");
  auto queryOp4B = resource->queryOperation("'NodeB' [ string { /f.o/ }]");
  auto queryOp5B = resource->queryOperation("'NodeB' [ string { /f.o/ = /b.r/ } ]");
  auto queryOp6B = resource->queryOperation("'NodeB' [ string { /f.o/ = /c.r/ } ]");
  auto queryOp7B = resource->queryOperation("'NodeB' [ floating-point { /f.o/ }]");
  auto queryOp8B = resource->queryOperation("'NodeB' [ floating-point { /f.o/ = 3.14159 } ]");
  auto queryOp9B = resource->queryOperation("'NodeB' [ floating-point { /f.o/ = 2.71828 } ]");

  std::array<smtk::resource::Component*, 2> components = { nodeA.get(), nodeB.get() };

  std::array<decltype(queryOp1A)*, 9> queryOpsA = { &queryOp1A, &queryOp2A, &queryOp3A,
                                                    &queryOp4A, &queryOp5A, &queryOp6A,
                                                    &queryOp7A, &queryOp8A, &queryOp9A };
  std::array<decltype(queryOp1A)*, 9> queryOpsB = { &queryOp1B, &queryOp2B, &queryOp3B,
                                                    &queryOp4B, &queryOp5B, &queryOp6B,
                                                    &queryOp7B, &queryOp8B, &queryOp9B };

  std::array<decltype(queryOpsA)*, 2> queryOps = { &queryOpsA, &queryOpsB };

  for (int i = 0; i < 9; ++i)
  {
    for (int j = 0; j < 2; ++j)
    {
      for (int k = 0; k < 2; ++k)
      {
        bool expected = (j == k ? (i % 3 != 2) : false);
        test(
          (*(*queryOps[j])[i])(*components[k]) == expected,
          "Filter operation " + std::to_string(i) + ", " + std::to_string(j) +
            " returned unexpected result for component " + std::to_string(k));
      }
    }
  }

  return 0;
}
