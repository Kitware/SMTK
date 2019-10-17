//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/view/Registrar.h"

#include "smtk/view/ComponentPhraseModel.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SelectionPhraseModel.h"

#include <tuple>

namespace smtk
{
namespace view
{
namespace
{
typedef std::tuple<PhraseModel, ResourcePhraseModel, ComponentPhraseModel, SelectionPhraseModel>
  PhraseModelList;
}
// TODO subphraseGenerators

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->registerPhraseModels<PhraseModelList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->unregisterPhraseModels<PhraseModelList>();
}
}
}
