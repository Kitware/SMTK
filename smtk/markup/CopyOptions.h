//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_CopyOptions_h
#define smtk_markup_CopyOptions_h

#include "smtk/markup/Exports.h"
#include "smtk/resource/CopyOptions.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT CopyOptions
{
public:
  using CopyType = smtk::resource::CopyOptions::CopyType;

  CopyType copyDiscreteData() const { return m_copyDiscreteData; }
  bool setCopyDiscreteData(CopyType);

protected:
  CopyType m_copyDiscreteData{ CopyType::Shallow };
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_CopyOptions_h
