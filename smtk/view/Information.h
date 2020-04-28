//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Information_h
#define smtk_view_Information_h

#include "smtk/CoreExports.h"

namespace smtk
{
namespace view
{
class Configuration;

/**\brief A base class for information passed to views during initialization.
  *
  * View information must include configuration information, but usually
  * also includes information specific to the GUI system of the view
  * being constructed. Hence, this class is usually dynamically cast to
  * a type appropriate to the view.
  */
class SMTKCORE_EXPORT Information
{
public:
  virtual ~Information() = 0;

  virtual const Configuration* configuration() const = 0;
};
}
}

#endif
