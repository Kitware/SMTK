//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ComponentsByTypeGenerator_h
#define smtk_view_ComponentsByTypeGenerator_h

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

class SMTKCORE_EXPORT ComponentsByTypeGenerator : public SubphraseGenerator
{
public:
  smtkTypeMacro(EntityTypeSubphrases);
  smtkSuperclassMacro(SubphraseGenerator);
  smtkSharedPtrCreateMacro(SubphraseGenerator);
  virtual ~ComponentsByTypeGenerator();

  DescriptivePhrases subphrases(DescriptivePhrase::Ptr src) override;
  bool shouldOmitProperty(DescriptivePhrase::Ptr parent, smtk::resource::PropertyType ptype,
    const std::string& pname) const override;

protected:
  ComponentsByTypeGenerator();

  virtual void childrenOfComponent(EntityPhrase::Ptr, DescriptivePhrases&);
  virtual void childrenOfComponentList(EntityListPhrase::Ptr, DescriptivePhrases&);

  bool m_abridgeUses;
};
}
}

#endif
