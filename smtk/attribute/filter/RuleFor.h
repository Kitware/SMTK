//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_RuleFor_h
#define smtk_attribute_filter_RuleFor_h

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"

#include "smtk/resource/PersistentObject.h"
#include "smtk/resource/filter/Rule.h"

#include <algorithm>

namespace smtk
{
namespace attribute
{
namespace filter
{

///\brief Filter Grammar Rules for dealing with filtering Attributes and Definitions
///
/// A class template based on smtk::resource::filter::RuleFor.  This implementation
/// provides support for rules dealing with a specific property type that take into
/// consideration the relationship between Attributes and Definitions.  For filtering
/// purposes an attribute will inherit properties from it Definition in terms of names and
/// values.  An Attribute will override a value for a particular property type/name if it
/// explicitly as the same property name and type.  Similarity, a Definition will inherit
/// properties from its Base Definition.
template<typename Type>
class RuleFor : public smtk::resource::filter::Rule
{
public:
  RuleFor()
    : acceptableKeys(
        [](const smtk::resource::PersistentObject&) { return std::vector<std::string>(); })
    , acceptableValue([](const Type&) { return true; })
  {
  }

  ~RuleFor() override = default;

  bool operator()(const smtk::resource::PersistentObject& object) const override
  {
    auto acceptable = acceptableKeys(object);
    // First check to see if the object itself matches the criteria
    for (auto it = acceptable.begin(); it != acceptable.end();)
    {
      if (object.properties().contains<Type>(*it))
      {
        if (acceptableValue(object.properties().at<Type>(*it)))
        {
          return true;
        }
        // Else remove the key since this key overrides anything coming from
        // its definitions
        it = acceptable.erase(it);
      }
      else
      {
        // skip it
        ++it;
      }
    }
    // Ok since the object itself didn't match the criteria, lets test its definitions
    smtk::attribute::DefinitionPtr def;

    const auto* att = dynamic_cast<const smtk::attribute::Attribute*>(&object);
    if (att)
    {
      def = att->definition();
    }
    else
    {
      const auto* attDef = dynamic_cast<const smtk::attribute::Definition*>(&object);
      if (attDef)
      {
        def = attDef->baseDefinition();
      }
      else
      {
        return false;
      }
    }
    for (; def != nullptr; def = def->baseDefinition())
    {
      for (auto it = acceptable.begin(); it != acceptable.end();)
      {
        if (def->properties().contains<Type>(*it))
        {
          if (acceptableValue(def->properties().at<Type>(*it)))
          {
            return true;
          }
          // Else remove the key since this key overrides anything coming from
          // its base definition
          it = acceptable.erase(it);
        }
        else
        {
          // skip it
          ++it;
        }
      }
    }
    return false;
  }

  // Given a persistent object, return a vector of keys that match the
  // name filter.
  std::function<std::vector<std::string>(const smtk::resource::PersistentObject&)> acceptableKeys;

  // Given a value, determine whether this passes the filter.
  std::function<bool(const Type&)> acceptableValue;
};
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
