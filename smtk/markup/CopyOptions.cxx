//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/CopyOptions.h"

namespace smtk
{
namespace markup
{

bool CopyOptions::setCopyDiscreteData(CopyType shouldCopy)
{
  if (shouldCopy == m_copyDiscreteData)
  {
    return false;
  }
  m_copyDiscreteData = shouldCopy;
  return true;
}

} // namespace markup
} // namespace smtk
