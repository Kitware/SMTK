//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/PhraseModelFactory.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

std::shared_ptr<PhraseModel> PhraseModelFactory::createFromConfiguration(
  const Configuration* config)
{
  std::shared_ptr<PhraseModel> phraseModel;

  if (!config)
  {
    return phraseModel;
  }

  // Find type typename of the phrase model to create from the configuration.
  const auto& topComp = config->details();
  int phraseModelIndex = topComp.findChild("PhraseModel");
  if (phraseModelIndex < 0)
  {
    return phraseModel;
  }
  const auto& phraseComp = topComp.child(phraseModelIndex);
  std::string typeName;
  if (!phraseComp.attribute("Type", typeName))
  {
    return phraseModel;
  }

  phraseModel = this->createFromName(typeName, config, m_manager);
  if (phraseModel)
  {
    // This must be called *after* construction, since weak pointers
    // cannot be created inside an object's constructor:
    phraseModel->root()->findDelegate()->setModel(phraseModel);
  }
  return phraseModel;
}
}
}
