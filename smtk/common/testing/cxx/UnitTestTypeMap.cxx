//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/TypeMap.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"
#include "smtk/common/json/jsonTypeMap.h"
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

class TypeMap : public smtk::common::TypeMap<std::string>
{
public:
  template<typename Type>
  void insertType()
  {
    return smtk::common::TypeMap<std::string>::insertType<Type>();
  }

  template<typename Tuple>
  void insertTypes()
  {
    smtk::common::TypeMap<std::string>::insertTypes<Tuple>();
  }
};
} // namespace

int UnitTestTypeMap(int /*unused*/, char** const /*unused*/)
{
  TypeMap typeMap;

  test(!typeMap.contains<int>("foo"), "New instance should contain no values.");

  try
  {
    typeMap.emplace("foo", 3);
    test(false, "Insertion of a type prior to type insertion should throw an exception.");
  }
  catch (const std::domain_error&)
  {
  }

  typeMap.insertType<int>();

  typeMap.emplace("foo", 3);
  test(typeMap.at<int>("foo") == 3, "Inserted value should be retrievable.");

  bool inserted = typeMap.insert<int>("foo", 4);
  test(!inserted, "Second insertion should not succeed.");

  const smtk::common::TypeMapEntry<std::string, int>& constIntTypeMap = typeMap.get<int>();

  test(
    constIntTypeMap.at("foo") == 3,
    "Inserted value should be accessible via specialized interface.");

  try
  {
    smtk::common::TypeMapEntry<std::string, float>& floatTypeMap = typeMap.get<float>();
    test(false, "Access of a type prior to type insertion should throw an exception.");
    (void)floatTypeMap;
  }
  catch (const std::domain_error&)
  {
  }

  typeMap.insertType<float>();

  smtk::common::TypeMapEntry<std::string, float>& floatTypeMap = typeMap.get<float>();

  floatTypeMap["foo"] = 2.3f;

  test(
    fabs(floatTypeMap.at("foo") - 2.3f) < float_epsilon,
    "Assignment should be accessible via specialized interface.");

  test(
    fabs(typeMap.at<float>("foo") - 2.3f) < float_epsilon,
    "Assigned values should be accessible via generalized interface.");

  typedef std::unordered_map<smtk::common::UUID, std::vector<std::string>> StringMap;
  typeMap.insertType<StringMap>();
  smtk::common::TypeMapEntry<std::string, StringMap>& stringMapTypeMap = typeMap.get<StringMap>();

  smtk::common::UUID id = smtk::common::UUID::random();
  stringMapTypeMap["foo"][id] = { "zero", "one", "two", "three" };

  test(stringMapTypeMap["foo"][id][0] == "zero", "Composite types should be assignable.");

  typeMap.insertType<Foo>();
  typeMap.get<Foo>().insert("foo", Foo(3));

  test(typeMap.at<Foo>("foo").m_i == 3, "Custom types should be insertable.");

  nlohmann::json j = typeMap;

  std::string key = smtk::common::typeName<Foo>();
  test(
    j.find(key) == j.end(), "Types without serialization should be omitted from the json stream.");

  typedef std::tuple<int, float, StringMap> TypeList;

  std::cout << std::endl;

  TypeMap typeMap2;
  typeMap2.insertTypes<TypeList>();
  smtk::common::from_json(j, typeMap2);

  nlohmann::json j2 = typeMap2;

  test(j == j2, "Serialized typeMap should match original typeMap.");

  return 0;
}
