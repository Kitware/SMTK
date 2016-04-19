//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/extension/qt/qtUIManager.h"

SMTKViewConstructorMap qtSMTKUtilities::m_viewConstructors;

const SMTKViewConstructorMap& qtSMTKUtilities::viewConstructors()
{
  return qtSMTKUtilities::m_viewConstructors;
}

void qtSMTKUtilities::registerViewConstructor(const std::string& viewname, qtSMTKViewConstructor viewc)
{
  // this will overwrite the existing constructor if the viewname exists in the map
  qtSMTKUtilities::m_viewConstructors[viewname] = viewc;
}

void qtSMTKUtilities::updateViewConstructors(smtk::attribute::qtUIManager* uiMan)
{
  if(!uiMan || qtSMTKUtilities::viewConstructors().size() == 0)
    return;

  SMTKViewConstructorMap::const_iterator it;
  for(it = qtSMTKUtilities::viewConstructors().begin();
      it != qtSMTKUtilities::viewConstructors().end(); ++it)
    {
    uiMan->registerViewConstructor(it->first, it->second);
    }
}
