//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_appcomponents_VisibleResourcePhraseModel_h
#define smtk_extension_appcomponents_VisibleResourcePhraseModel_h

#include "smtk/view/ResourcePhraseModel.h"

namespace smtk
{
namespace view
{

/**\brief A subclass of ResourcePhraseModel that decorates items with ParaView visibility.
  *
  */
class VisibleResourcePhraseModel : public ResourcePhraseModel
{
public:
  smtkTypeMacro(VisibleResourcePhraseModel);
  smtkSuperclassMacro(ResourcePhraseModel);
  smtkSharedPtrCreateMacro(PhraseModel);
  virtual ~VisibleResourcePhraseModel();

  void decoratePhrase(DescriptivePhrasePtr phr) const override;

  void setResourceManager(pqResourceManager* rsrcMgr);

protected:
  VisibleResourcePhraseModel();

  pqResourceManager* m_pqResourceManager;
};
}
}

#endif
