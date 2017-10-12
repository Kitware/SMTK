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
#include "smtk/model/EntityTypeBits.h"

#include <QActionGroup>

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSelectionFilterBehavior : public QActionGroup
{
  Q_OBJECT
  using Superclass = QActionGroup;

public:
  pqSMTKSelectionFilterBehavior(
    QObject* parent = nullptr, smtk::resource::SelectionManagerPtr mgr = nullptr);
  ~pqSMTKSelectionFilterBehavior() override;

  static pqSMTKSelectionFilterBehavior* instance();

  void setSelectionManager(smtk::resource::SelectionManagerPtr selnMgr);

protected slots:
  virtual void onFilterChanged(QAction* a);

protected:
  class pqInternal;
  pqInternal* m_p;
  smtk::model::BitFlags m_modelFilterMask;
  smtk::resource::SelectionManagerPtr m_selectionManager;

private:
  Q_DISABLE_COPY(pqSMTKSelectionFilterBehavior);
};
