//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_view_SubphraseGenerator_h
#define smtk_project_view_SubphraseGenerator_h

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace project
{
namespace view
{

/// Generate subphrases to display for project descriptive phrases.
class SMTKCORE_EXPORT SubphraseGenerator : public smtk::view::SubphraseGenerator
{
public:
  smtkTypeMacro(smtk::project::view::SubphraseGenerator);
  smtkSuperclassMacro(smtk::view::SubphraseGenerator);
  smtkSharedPtrCreateMacro(smtk::view::SubphraseGenerator);

  SubphraseGenerator();
  ~SubphraseGenerator() override;

  /// Return a list of descriptive phrases that elaborate upon \a src.
  smtk::view::DescriptivePhrases subphrases(smtk::view::DescriptivePhrase::Ptr src) override;

  void childrenOfProject(
    smtk::view::DescriptivePhrase::Ptr src,
    smtk::project::ProjectPtr project,
    smtk::view::DescriptivePhrases& result);

protected:
  Path indexOfObjectInParent(
    const smtk::resource::PersistentObjectPtr& obj,
    const smtk::view::DescriptivePhrasePtr& parent,
    const Path& parentPath) override;
};
} // namespace view
} // namespace project
} // namespace smtk

#endif
