//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/CopyAssignmentOptions.h"

#include <sstream>

using namespace smtk::attribute;

std::string AttributeCopyOptions::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix << (!this->performAssignment() ? " !" : " ") << "PerformAssignment" << std::endl;
  ss << prefix << (!this->copyUUID() ? " !" : " ") << "CopyUUID" << std::endl;
  ss << prefix << (!this->copyDefinition() ? " !" : " ") << "CopyDefinition" << std::endl;
  return ss.str();
}

std::string CommonAssignmentOptions::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix << (!this->m_objectMapping ? " No " : " ") << "Mapping Provided" << std::endl;
  return ss.str();
}

std::string AttributeAssignmentOptions::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix << (!this->ignoreMissingItems() ? " !" : " ") << "IgnoreMissingItems" << std::endl;
  ss << prefix << (!this->copyAssociations() ? " !" : " ") << "CopyAssociations" << std::endl;
  ss << prefix << (!this->allowPartialAssociations() ? " !" : " ") << "AllowPartialAssociations"
     << std::endl;
  ss << prefix << (!this->doNotValidateAssociations() ? " !" : " ") << "DoNotValidateAssociations"
     << std::endl;
  ss << CommonAssignmentOptions::convertToString(prefix);
  return ss.str();
}

std::string ItemAssignmentOptions::convertToString(const std::string& prefix) const
{
  std::stringstream ss;
  ss << prefix << (!this->ignoreMissingChildren() ? " !" : " ") << "IgnoreMissingChildren"
     << std::endl;
  ss << prefix << (!this->allowPartialValues() ? " !" : " ") << "AllowPartialValues" << std::endl;
  ss << prefix << (!this->ignoreExpressions() ? " !" : " ") << "IgnoreExpressions" << std::endl;
  ss << prefix << (!this->ignoreReferenceValues() ? " !" : " ") << "IgnoreReferenceValues"
     << std::endl;
  ss << prefix << (!this->doNotValidateReferenceInfo() ? " !" : " ") << "DoNotValidateReferenceInfo"
     << std::endl;
  ss << prefix << (!this->disableCopyAttributes() ? " !" : " ") << "DisableCopyAttributes"
     << std::endl;
  ss << CommonAssignmentOptions::convertToString(prefix);
  return ss.str();
}

std::string CopyAssignmentOptions::convertToString(const std::string& prefix) const
{
  std::string myPrefix = prefix + "\t";
  std::stringstream ss;
  ss << prefix << "Copy Options:\n" << copyOptions.convertToString(myPrefix) << std::endl;
  ss << prefix << "Attribute Options:\n" << attributeOptions.convertToString(myPrefix) << std::endl;
  ss << prefix << "Item Options:\n" << itemOptions.convertToString(myPrefix) << std::endl;
  return ss.str();
}
