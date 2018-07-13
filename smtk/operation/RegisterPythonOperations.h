//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_RegisterPythonOperations_h
#define __smtk_model_RegisterPythonOperations_h

#include "smtk/operation/Manager.h"

namespace smtk
{
namespace operation
{

bool registerPythonOperations(
  const smtk::operation::Manager::Ptr& operationManager, const std::string& moduleName);
}
}

#endif
