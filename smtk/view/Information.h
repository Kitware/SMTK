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

#include "smtk/common/TypeContainer.h"

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/view/Configuration.h"

namespace smtk
{
namespace view
{

/**\brief A  class for information passed to views during initialization.
  *
  * View information must include configuration information, but usually
  * also includes information specific to the GUI system of the view
  * being constructed. Hence, this class is based off of TypeContainer so it
  * can hold arbitrary information.
  */
class SMTKCORE_EXPORT Information
  : public smtk::common::TypeContainer
  , public std::enable_shared_from_this<Information>
{
public:
  typedef TypeContainer Container;

  smtkTypeMacroBase(smtk::view::Information);
  smtkCreateMacro(Information);

  Information() = default;
  ~Information() override;

  virtual const Configuration* configuration() const
  {
    return this->get<smtk::view::ConfigurationPtr>().get();
  }

protected:
};

} // namespace view
} // namespace smtk

#endif
