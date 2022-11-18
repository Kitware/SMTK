//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_SubphraseGenerator_h
#define smtk_markup_SubphraseGenerator_h

#include "smtk/markup/Exports.h"

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT SubphraseGenerator : public smtk::view::SubphraseGenerator
{
public:
  smtkTypeMacro(smtk::markup::SubphraseGenerator);
  smtkCreateMacro(smtk::view::SubphraseGenerator);
  smtkSuperclassMacro(smtk::view::SubphraseGenerator);

  using PhrasePath = Superclass::Path;

  SubphraseGenerator();
  ~SubphraseGenerator() override = default;

  /**\brief Return a list of descriptive phrases that elaborate upon \a parent.
    *
    * Subclasses must override this method.
    */
  smtk::view::DescriptivePhrases subphrases(smtk::view::DescriptivePhrase::Ptr parent) override;

  /**\brief Return true if children would be generated for the Descriptive Phrase.
   */
  bool hasChildren(const smtk::view::DescriptivePhrase& parent) const override;

  /**\brief Return a set of parent Persistent Objects for this object.
   * based on the generator's parent/child rules
   */
  smtk::resource::PersistentObjectSet parentObjects(
    const smtk::resource::PersistentObjectPtr& obj) const override;

  /**\brief Append subphrases and their paths that the given set of created objects implies.
    *
    * After an operation, newly-created objects (components and resources) need to be
    * inserted into the descriptive phrase hierarchy. Since the subphrase generator is
    * responsible for populating all of the tree except the top-level phrases initially,
    * this task also falls to the generator.
    *
    * The generator is responsible for decorating each path it adds to \a resultingPhrases
    * if a phrase model is present.
    */
  void subphrasesForCreatedObjects(
    const smtk::resource::PersistentObjectArray& objects,
    const smtk::view::DescriptivePhrasePtr& localRoot,
    PhrasesByPath& resultingPhrases) override;

protected:
  PhrasePath indexOfObjectInParent(
    const smtk::resource::PersistentObjectPtr& obj,
    const smtk::view::DescriptivePhrasePtr& parent,
    const PhrasePath& parentPath) override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_SubphraseGenerator_h
