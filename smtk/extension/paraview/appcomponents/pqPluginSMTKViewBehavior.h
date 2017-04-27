//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_pq_PluginSMTKViewBehavior_h
#define __smtk_pq_PluginSMTKViewBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include <QObject>

/// @ingroup Behaviors
/// pqPluginSMTKViewBehavior adds support for loading user-fined views from
/// plugins. In other words, it adds support customized ui for views created using
/// ADD_SMTK_UI_VIEW.
class SMTKPQCOMPONENTSEXT_EXPORT pqPluginSMTKViewBehavior : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqPluginSMTKViewBehavior(QObject* p);

public slots:
  void addPluginInterface(QObject* iface);

private:
  Q_DISABLE_COPY(pqPluginSMTKViewBehavior)
};

#endif
