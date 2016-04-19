//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_qtSMTKUtilities_h
#define __smtk_attribute_qtSMTKUtilities_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtViewInterface.h" // for qtSMTKViewConstructor definition

#include <map>

typedef std::map<std::string, qtSMTKViewConstructor> SMTKViewConstructorMap;
/// qtSMTKUtilities is a collection of arbitrary utility functions that can be
/// used by an smtk application.
class SMTKQTEXT_EXPORT qtSMTKUtilities 
{

public:

  static const SMTKViewConstructorMap& viewConstructors();

  // this will overwrite the existing constructor if the viewname exists in the map
  static void registerViewConstructor(const std::string& viewname, qtSMTKViewConstructor viewc);

  static void updateViewConstructors(smtk::attribute::qtUIManager* uiMan);

private:
  static SMTKViewConstructorMap m_viewConstructors;
};

#endif
