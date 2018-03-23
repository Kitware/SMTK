//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_environment_Environment_h
#define __smtk_attribute_environment_Environment_h

#include "smtk/attribute/environment/Exports.h"

namespace smtk
{
namespace attribute
{
namespace environment
{
struct SMTKATTRIBUTEENVIRONMENT_EXPORT RegisterToEnvironment
{
  RegisterToEnvironment();
  ~RegisterToEnvironment();

  RegisterToEnvironment(const RegisterToEnvironment&) = delete;
  RegisterToEnvironment& operator=(const RegisterToEnvironment&) = delete;
};

// Uses schwartz counter idiom for management
static RegisterToEnvironment registerToEnvironment;
}
}
}

#endif
