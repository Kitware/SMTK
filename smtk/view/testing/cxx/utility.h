//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_testing_cxx_utility_h
#define smtk_view_testing_cxx_utility_h

#include "smtk/view/ResourcePhraseModel.h"

namespace smtk
{
namespace view
{

PhraseModel::Ptr loadTestData(
  int argc,
  char* argv[],
  const ManagerPtr& viewManager,
  const Configuration& config,
  std::vector<char*>& dataArgs,
  smtk::resource::ManagerPtr& resourceManager,
  smtk::operation::ManagerPtr& operationManager);
}
} // namespace smtk

#endif // smtk_view_testing_cxx_utility_h
