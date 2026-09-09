//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtAttributeItemInfo.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/view/Configuration.h"

int UnitTestQtAttributeItemInfo(int, char** const)
{
  smtk::extension::qtAttributeItemInfo info;

  test(info.indentChildren(), "Children should be indented by default.");

  smtk::view::Configuration::Component component;
  component.setAttribute("IndentChildren", "false");
  info.setComponent(component);
  test(!info.indentChildren(), "IndentChildren=false should disable child indentation.");

  component.setAttribute("IndentChildren", "true");
  info.setComponent(component);
  test(info.indentChildren(), "IndentChildren=true should enable child indentation.");

  return 0;
}
