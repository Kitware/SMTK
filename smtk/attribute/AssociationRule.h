//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_AssociationRule_h
#define __smtk_attribute_AssociationRule_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/Attribute.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace attribute
{

/// A base class for custom (i.e. user-defined) association/dissociation rules.
/// This class defines the requisite API for custom attribute rules.
class SMTKCORE_EXPORT Rule
{
public:
  smtkTypenameMacroBase(smtk::attribute::Rule);

  virtual ~Rule() = default;

  /// Verify whether an attribute can be associated/dissociated to/from a
  /// persistent object.
  virtual bool operator()(
    const Attribute::ConstPtr&, const smtk::resource::PersistentObject::ConstPtr&) const = 0;

  /// Serialize the rule to/from json.
  virtual const Rule& operator>>(nlohmann::json& json) const = 0;
  virtual Rule& operator<<(const nlohmann::json& json) = 0;

  /// Serialize the rule to/from xml.
  virtual const Rule& operator>>(pugi::xml_node& node) const = 0;
  virtual Rule& operator<<(const pugi::xml_node& node) = 0;
};

/// A subclass used to differentiate between rule types during registration.
/// Association rules can inherit from this class, but it suffices to have them
/// inherit from the base Rule class.
class SMTKCORE_EXPORT AssociationRule : public Rule
{
};

/// A subclass used to differentiate between rule types during registration.
/// Dissociation rules can inherit from this class, but it suffices to have them
/// inherit from the base Rule class.
class SMTKCORE_EXPORT DissociationRule : public Rule
{
};
}
}

#endif
