//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_AutoStart_h
#define smtk_extension_paraview_project_AutoStart_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include <QObject>

/// Application-level modifications to accommodate Projects.
///
/// Currently, when the Projects plugin is loaded, we locate the resource browser
/// and add a filter to remove Project Resources from it.
class SMTKPQPROJECTEXT_EXPORT pqSMTKProjectAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKProjectAutoStart(QObject* parent = nullptr);
  ~pqSMTKProjectAutoStart() override;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(pqSMTKProjectAutoStart);
};

#endif
