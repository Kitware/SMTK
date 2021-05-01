//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_plugin_AutoStart_h
#define smtk_project_plugin_AutoStart_h

#include <QObject>

/// Application-level modifications to accommodate Projects.
///
/// Currently, when the Projects plugin is loaded, we locate the resource browser
/// and add a filter to remove Project Resources from it.
class AutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  AutoStart(QObject* parent = nullptr);
  ~AutoStart() override;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(AutoStart);
};

#endif
