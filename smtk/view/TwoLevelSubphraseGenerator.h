//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_TwoLevelSubphraseGenerator_h
#define smtk_view_TwoLevelSubphraseGenerator_h

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

/**\brief Generate subphrases in a relatively flat two-level hierarchy.
  *
  */
class SMTKCORE_EXPORT TwoLevelSubphraseGenerator : public SubphraseGenerator
{
public:
  smtkTypeMacro(smtk::view::TwoLevelSubphraseGenerator);
  smtkSuperclassMacro(smtk::view::SubphraseGenerator);
  smtkSharedPtrCreateMacro(smtk::view::SubphraseGenerator);
  TwoLevelSubphraseGenerator();
  virtual ~TwoLevelSubphraseGenerator();

  /**\brief Return a list of descriptive phrases that elaborate upon \a src.
    *
    * Subclasses must override this method.
    */
  DescriptivePhrases subphrases(DescriptivePhrase::Ptr src) override;

protected:
  bool findSortedLocation(
    Path& pathInOut,
    smtk::model::EntityPtr entity,
    DescriptivePhrase::Ptr& phr,
    const DescriptivePhrase::Ptr& parent) const override;

  void childrenOfResource(
    DescriptivePhrase::Ptr src,
    smtk::resource::ResourcePtr rsrc,
    DescriptivePhrases& result);
};
} // namespace view
} // namespace smtk

#endif // smtk_view_TwoLevelSubphraseGenerator_h
