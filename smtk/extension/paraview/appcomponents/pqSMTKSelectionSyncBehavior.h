//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include <QObject>

class pqOutputPort;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSelectionSyncBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKSelectionSyncBehavior(
    QObject* parent = nullptr, smtk::resource::SelectionManagerPtr mgr = nullptr);
  ~pqSMTKSelectionSyncBehavior() override;

  smtk::resource::SelectionManagerPtr selectionManager() const { return m_selectionManager; }

protected slots:
  virtual void connectSelectionManagers();
  virtual void paraviewSelectionChanged(pqOutputPort* port);

protected:
  bool m_connected;
  smtk::resource::SelectionManagerPtr m_selectionManager;
  std::string m_selectionSource;
  int m_selectedValue;
  int m_hoveredValue;
  int m_selectionListener;

private:
  Q_DISABLE_COPY(pqSMTKSelectionSyncBehavior);
};
