//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_qtViewInterface_h
#define __smtk_attribute_qtViewInterface_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

#include <QString>
#include <QtPlugin>

typedef smtk::attribute::qtBaseView* (*qtSMTKViewConstructor)(const smtk::attribute::ViewInfo &info);

namespace smtk
{
  namespace attribute
  {

  /// interface class for plugins that add a QDockWindow
  class SMTKQTEXT_EXPORT qtViewInterface
  {
  public:
    qtViewInterface();
    virtual ~qtViewInterface();

    virtual QString viewName() const = 0;

    /// return a static constructor for derived class of qtBaseView 
    virtual qtSMTKViewConstructor viewConstructor() = 0;
  private:
    Q_DISABLE_COPY(qtViewInterface)
  };

  }; // namespace attribute
}; // namespace smtk

Q_DECLARE_INTERFACE(smtk::attribute::qtViewInterface, "com.kitware/paraview/smtkview")

#endif
