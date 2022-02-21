//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#ifndef pqSlicePropertyWidgetBaseP_h
#define pqSlicePropertyWidgetBaseP_h

#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"
#include "ui_pqSlicePropertyWidgetBase.h"

#include "pqDisplayColorWidget.h"
#include "pqPipelineSource.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMProxy.h"
#include "vtkSMTransferFunctionManager.h"

#include "vtkNew.h"

#include <QCheckBox>
#include <QColor>
#include <QPointer>
#include <QWidget>

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS public
#define Q_OBJECT
#endif

class pqSlicePropertyWidgetBase;

/**
 * pqSlicePropertyWidgetBaseP is an internal helper that holds Qt widgets
 * and other state for pqSlicePropertyWidgetBase and its subclasses.
 * Because subclasses need access to this information, the class is
 * declared in a public header.
 */
class SMTKPQWIDGETSEXT_EXPORT pqSlicePropertyWidgetBaseP : public Ui::SlicePropertyWidgetBase
{
public:
  pqSlicePropertyWidgetBaseP(
    pqSlicePropertyWidgetBase* self,
    pqPipelineSource* inputData,
    vtkSMProxy* smproxy,
    vtkSMPropertyGroup* smgroup,
    QWidget* parentObject);

  QPointer<pqPipelineSource> input;
  QPointer<pqDisplayColorWidget> colorBy;
  vtkNew<vtkSMTransferFunctionManager> transferFunctions;
};

#endif
