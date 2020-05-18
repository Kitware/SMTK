//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_QueryFilterSubphraseGenerator_h
#define smtk_view_QueryFilterSubphraseGenerator_h

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

/**\brief Generate subphrases by querying the filter in ObjectGroupPhraseContent
  *
  */
class SMTKCORE_EXPORT QueryFilterSubphraseGenerator : public SubphraseGenerator
{
public:
  smtkTypeMacro(smtk::view::QueryFilterSubphraseGenerator);
  smtkSuperclassMacro(smtk::view::SubphraseGenerator);
  smtkSharedPtrCreateMacro(smtk::view::SubphraseGenerator);
  QueryFilterSubphraseGenerator();
  virtual ~QueryFilterSubphraseGenerator();

  /**\brief Return a list of descriptive phrases that elaborate upon \a src.
    *
    */
  DescriptivePhrases subphrases(DescriptivePhrase::Ptr src) override;

protected:
  Path indexOfObjectInParent(
    const smtk::resource::PersistentObjectPtr& obj,
    smtk::view::DescriptivePhrasePtr& parent,
    const Path& parentPath) override;
};
} // namespace view
} // namespace smtk

#endif // smtk_view_TwoLevelSubphraseGenerator_h
