//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_VisibilityContent_h
#define smtk_view_VisibilityContent_h

#include "smtk/view/PhraseContent.h"

#include <functional>

namespace smtk
{
namespace view
{

/**\brief Describe a resource component for user presentation.
  *
  */
class SMTKCORE_EXPORT VisibilityContent : public PhraseContent
{
public:
  /// Types of queries for visibility.
  enum Query
  {
    DISPLAYABLE,
    EDITABLE,
    GET_VALUE,
    SET_VALUE
  };
  /// Signature of a function to inspect or set the subject's visibility.
  using Delegate = std::function<int(Query q, int v, ConstPhraseContentPtr data)>;

  smtkTypeMacro(VisibilityContent);
  smtkSuperclassMacro(PhraseContent);
  smtkSharedPtrCreateMacro(PhraseContent);
  virtual ~VisibilityContent();
  Ptr setup(Delegate delegate);

  static DescriptivePhrasePtr decoratePhrase(DescriptivePhrasePtr phr, Delegate delegate);

  bool displayable(ContentType attr) const override;
  bool editable(ContentType attr) const override;

  int flagValue(ContentType attr) const override;

  bool editFlagValue(ContentType attr, int val) override;

  bool operator==(const PhraseContent& other) const override
  {
    return this->equalTo(other); // no way to compare m_delegate members.
  }

protected:
  VisibilityContent();

  Delegate m_delegate;
};

} // view namespace
} // smtk namespace

#endif
