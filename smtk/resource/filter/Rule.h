//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Rule_h
#define smtk_resource_filter_Rule_h

#include "smtk/resource/PersistentObject.h"

#include <algorithm>

namespace smtk
{
namespace resource
{
namespace filter
{

/// A base class for filter rules.
class Rule
{
public:
  virtual ~Rule() = default;

  virtual bool operator()(const PersistentObject&) const = 0;
};

/// A class template for rules dealing with a specific property type.
template<typename Type>
class RuleFor : public Rule
{
public:
  RuleFor()
    : acceptableKeys([](const PersistentObject&) { return std::vector<std::string>(); })
    , acceptableValue([](const Type&) { return true; })
  {
  }

  ~RuleFor() override = default;

  bool operator()(const PersistentObject& object) const override
  {
    auto acceptable = acceptableKeys(object);
    return std::any_of(
      acceptable.begin(), acceptable.end(), [this, &object](const std::string& key) {
        return acceptableValue(object.properties().at<Type>(key));
      });
  }

  // Given a persistent object, return a vector of keys that match the
  // name filter.
  std::function<std::vector<std::string>(const PersistentObject&)> acceptableKeys;

  // Given a value, determine whether this passes the filter.
  std::function<bool(const Type&)> acceptableValue;
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
