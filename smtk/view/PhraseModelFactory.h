//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseModelFactory_h
#define smtk_view_PhraseModelFactory_h

#include "smtk/common/Factory.h"
#include "smtk/view/PhraseModel.h"

namespace smtk
{
namespace view
{

class Configuration;
class Manager;

using PhraseModelParameters = smtk::common::factory::Inputs<const Configuration*, Manager*>;

/**\brief A factory to create phrase models which is held by view managers.
  *
  * This factory must be owned by a manager (referenced in its constructor)
  * and must not outlive its manager (or the pointer to the manager will
  * become stale).
  */
class SMTKCORE_EXPORT PhraseModelFactory
  : public smtk::common::Factory<PhraseModel, void, PhraseModelParameters>
{
public:
  PhraseModelFactory(Manager* manager)
    : m_manager(manager)
  {
  }

  virtual ~PhraseModelFactory() {}

  PhraseModelFactory() = delete;
  PhraseModelFactory(const PhraseModelFactory&) = delete;
  void operator=(const PhraseModelFactory&) = delete;

  /// Simplify creation by passing in the factory's view-manager.
  template <typename Class>
  std::unique_ptr<Class> create(const Configuration* config)
  {
    auto phraseModel = this->create<Class>(config, m_manager);
    if (phraseModel)
    {
      // This must be called *after* construction, since weak pointers
      // cannot be created inside an object's constructor:
      phraseModel->root()->findDelegate()->setModel(phraseModel);
    }
    return phraseModel;
  }

  std::shared_ptr<PhraseModel> createFromConfiguration(const Configuration* config);

protected:
  Manager* m_manager;
};
}
}

#endif
