//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_EmptySubphraseGenerator_h
#define smtk_view_EmptySubphraseGenerator_h

#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

/**\brief Generate no subphrases (but decorate root-level phrases).
  *
  */
class SMTKCORE_EXPORT EmptySubphraseGenerator : public SubphraseGenerator
{
public:
  smtkTypeMacro(smtk::view::EmptySubphraseGenerator);
  smtkSuperclassMacro(smtk::view::SubphraseGenerator);
  smtkSharedPtrCreateMacro(smtk::view::SubphraseGenerator);
  EmptySubphraseGenerator();
  virtual ~EmptySubphraseGenerator() {}

  /**\brief Return a list of descriptive phrases that elaborate upon \a src.
    *
    * Subclasses must override this method.
    */
  DescriptivePhrases subphrases(DescriptivePhrase::Ptr src) override;
};

} // namespace view
} // namespace smtk

#endif
