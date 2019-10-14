//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Properties.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"
#include "smtk/common/json/jsonProperties.h"
#include "smtk/common/json/jsonUUID.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <map>

namespace
{
const double float_epsilon = 1.e-6;

struct Foo
{
  Foo(int i)
    : m_i(i)
  {
  }
  int m_i;
};

class Properties : public smtk::common::Properties
{
public:
  template <typename Type>
  void insertPropertyType()
  {
    return smtk::common::Properties::insertPropertyType<Type>();
  }

  template <typename Tuple>
  void insertPropertyTypes()
  {
    smtk::common::Properties::insertPropertyTypes<Tuple>();
  }
};
}

int UnitTestProperties(int, char** const)
{
  Properties properties;

  test(properties.has<int>("foo") == false, "New instance should contain no values.");

  try
  {
    properties.emplace("foo", 3);
    test(false, "Insertion of a property type prior to type insertion should throw an exception.");
  }
  catch (const std::domain_error&)
  {
  }

  properties.insertPropertyType<int>();

  properties.emplace("foo", 3);
  test(properties.at<int>("foo") == 3, "Inserted value should be retrievable.");

  bool inserted = properties.insert<int>("foo", 4);
  test(inserted == false, "Second insertion should not succeed.");

  const smtk::common::PropertiesOfType<int>& constIntProperties = properties.get<int>();

  test(constIntProperties.at("foo") == 3,
    "Inserted value should be accessible via specialized interface.");

  try
  {
    smtk::common::PropertiesOfType<float>& floatProperties = properties.get<float>();
    test(false, "Access of a property type prior to type insertion should throw an exception.");
    (void)floatProperties;
  }
  catch (const std::domain_error&)
  {
  }

  properties.insertPropertyType<float>();

  smtk::common::PropertiesOfType<float>& floatProperties = properties.get<float>();

  floatProperties["foo"] = 2.3;

  test(fabs(floatProperties.at("foo") - 2.3) < float_epsilon,
    "Assignment should be accessible via specialized interface.");

  test(fabs(properties.at<float>("foo") - 2.3) < float_epsilon,
    "Assigned values should be accessible via generalized interface.");

  typedef std::unordered_map<smtk::common::UUID, std::vector<std::string> > StringMap;
  properties.insertPropertyType<StringMap>();
  smtk::common::PropertiesOfType<StringMap>& stringMapProperties = properties.get<StringMap>();

  smtk::common::UUID id = smtk::common::UUID::random();
  stringMapProperties["foo"][id] = { "zero", "one", "two", "three" };

  test(stringMapProperties["foo"][id][0] == "zero", "Composite types should be assignable.");

  properties.insertPropertyType<Foo>();
  properties.get<Foo>().insert("foo", Foo(3));

  test(properties.at<Foo>("foo").m_i == 3, "Custom types should be insertable.");

  nlohmann::json j = properties;

  std::string key = smtk::common::typeName<Foo>();
  test(
    j.find(key) == j.end(), "Types without serialization should be omitted from the json stream.");

  typedef std::tuple<int, float, StringMap> PropertyList;

  std::cout << std::endl;

  Properties properties2;
  properties2.insertPropertyTypes<PropertyList>();
  smtk::common::from_json(j, properties2);

  nlohmann::json j2 = properties2;

  test(j == j2, "Serialized properties should match original properties.");

  return 0;
}
