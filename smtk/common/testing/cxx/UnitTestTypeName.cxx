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

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <memory>

#include <unordered_map>
#include <vector>

class ConstExprNamed
{
public:
  static constexpr const char* const type_name = "ConstExprNamed";

  [[nodiscard]] std::string typeName() const { return "Should not return this"; }

  static std::shared_ptr<ConstExprNamed> create()
  {
    std::shared_ptr<ConstExprNamed> shared(new ConstExprNamed);
    return std::static_pointer_cast<ConstExprNamed>(shared);
  }
};

class FunctionNamed : public std::enable_shared_from_this<FunctionNamed>
{
public:
  std::string typeName() const { return "Should not return this"; }

  static std::shared_ptr<FunctionNamed> create()
  {
    std::shared_ptr<FunctionNamed> shared(new FunctionNamed);
    return std::static_pointer_cast<FunctionNamed>(shared);
  }
};

class Unnamed
{
};

namespace foo
{
namespace bar
{
class NestedClass
{
};
} // namespace bar
} // namespace foo

int UnitTestTypeName(int /*unused*/, char** const /*unused*/)
{
  using namespace std;

#define testNamed(CLASS_WITH_NAME)                                                                 \
  test(                                                                                            \
    smtk::common::typeName<CLASS_WITH_NAME>() == std::string(#CLASS_WITH_NAME),                    \
    std::string("Incorrect name for ") + std::string(#CLASS_WITH_NAME))

  testNamed(char);
  testNamed(unsigned char);
  testNamed(signed char);
  testNamed(int);
  testNamed(unsigned int);
  testNamed(short);
  testNamed(unsigned short);
  testNamed(long);
  testNamed(unsigned long);
  testNamed(float);
  testNamed(double);
  testNamed(long double);
  testNamed(wchar_t);

  testNamed(string);

  testNamed(ConstExprNamed);
  testNamed(FunctionNamed);
  testNamed(Unnamed);
  testNamed(foo::bar::NestedClass);

  testNamed(shared_ptr<FunctionNamed>);
  testNamed(weak_ptr<ConstExprNamed>);

  testNamed(vector<int>);
  testNamed(vector<double>);
  testNamed(vector<ConstExprNamed>);
  testNamed(vector<FunctionNamed>);

#undef testNamed

  test(
    smtk::common::typeName<tuple<int, double, ConstExprNamed, vector<FunctionNamed>>>() ==
      "tuple<int, double, ConstExprNamed, vector<FunctionNamed>>",
    "Tuple type name failed.");

  test(
    smtk::common::typeName<map<ConstExprNamed, set<vector<FunctionNamed>>>>() ==
      "map<ConstExprNamed, set<vector<FunctionNamed>>>",
    "Complex type name failed.");

  test(
    smtk::common::typeName<unordered_map<ConstExprNamed, set<vector<FunctionNamed>>>>() ==
      "unordered_map<ConstExprNamed, set<vector<FunctionNamed>>>",
    "unordered map type name failed.");

  return 0;
}
