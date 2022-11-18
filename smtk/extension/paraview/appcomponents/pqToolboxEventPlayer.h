//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_paraview_extension_appcomponents_pqToolboxEventPlayer_h
#define smtk_paraview_extension_appcomponents_pqToolboxEventPlayer_h

#include "pqWidgetEventPlayer.h"
#include "smtkPQComponentsExtModule.h"

/**
Concrete implementation of pqWidgetEventPlayer that handles playback of "activate" events for
buttons and menus.

\sa pqEventPlayer
*/
class SMTKPQCOMPONENTSEXT_EXPORT pqToolboxEventPlayer : public pqWidgetEventPlayer
{
  Q_OBJECT
  typedef pqWidgetEventPlayer Superclass;

public:
  pqToolboxEventPlayer(QObject* p = nullptr);

  using Superclass::playEvent;
  bool playEvent(QObject* Object, const QString& Command, const QString& Arguments, bool& Error)
    override;

private:
  pqToolboxEventPlayer(const pqToolboxEventPlayer&);
  pqToolboxEventPlayer& operator=(const pqToolboxEventPlayer&);
};

#endif // smtk_paraview_extension_appcomponents_pqToolboxEventPlayer_h
