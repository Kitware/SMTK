//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_view_IconConstructor_h
#define smtk_project_view_IconConstructor_h

#include "smtk/project/view/icons/project_svg.h"
#include "smtk/view/SVGIconConstructor.h"

namespace smtk
{
namespace project
{
namespace view
{

/**\brief An icon constructor for projects.
  *
  */
class SMTKCORE_EXPORT IconConstructor : public smtk::view::SVGIconConstructor
{
public:
  std::string operator()(const std::string& secondaryColor) const;

protected:
  std::string svg(const smtk::resource::PersistentObject&) const override { return project_svg; }
  std::string svg() const { return project_svg; }
};
} // namespace view
} // namespace project
} // namespace smtk

#endif
