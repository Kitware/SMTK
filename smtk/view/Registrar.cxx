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
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/SelectionPhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"

#include <tuple>

namespace smtk
{
namespace view
{
namespace
{
typedef std::tuple<PhraseModel, ResourcePhraseModel, ComponentPhraseModel, SelectionPhraseModel>
  PhraseModelList;
typedef std::tuple<SubphraseGenerator, TwoLevelSubphraseGenerator, EmptySubphraseGenerator>
  SubphraseGeneratorList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->registerPhraseModels<PhraseModelList>();
  viewManager->registerSubphraseGenerators<SubphraseGeneratorList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->unregisterPhraseModels<PhraseModelList>();
  viewManager->unregisterSubphraseGenerators<SubphraseGeneratorList>();
}
}
}
