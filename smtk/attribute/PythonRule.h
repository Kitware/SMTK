//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_PythonRule_h
#define __smtk_attribute_PythonRule_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/AssociationRule.h"

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <string>
#include <vector>

namespace pugi
{
class xml_node;
}

namespace smtk
{
namespace attribute
{

/// A custom rule type that evaluates a Python function to determine the
/// validity of association or dissociation.
class SMTKCORE_EXPORT PythonRule : public Rule
{
public:
  smtkTypenameMacro(smtk::attribute::PythonRule);

  bool operator()(
    const Attribute::ConstPtr&, const smtk::resource::PersistentObject::ConstPtr&) const override;

  const PythonRule& operator>>(nlohmann::json& json) const override;
  PythonRule& operator<<(const nlohmann::json& json) override;

  const PythonRule& operator>>(pugi::xml_node& node) const override;
  PythonRule& operator<<(const pugi::xml_node& node) override;

private:
  std::vector<std::string> m_sourceFiles;
  std::string m_functionString;
};
}
}

#endif
