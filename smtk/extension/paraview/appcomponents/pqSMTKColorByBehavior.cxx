//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKColorByBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKColorByWidget.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"

#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceManagerWrapper.h" // TODO: remove need for me

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/view/Selection.h"

#include "smtk/io/Logger.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqPVApplicationCore.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"

// Server side
#include "vtkPVSelectionSource.h"
#include "vtkSMSourceProxy.h"

// VTK
#include "vtkAbstractArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

// Qt
#include <QAction>
#include <QToolBar>
#include <QWidget>

// Qt generated UI
// #include "ui_pqSMTKColorByBehavior.h"

using namespace smtk;

#define NUM_ACTIONS 7

static pqSMTKColorByBehavior* s_colorByBar = nullptr;

class pqSMTKColorByBehavior::pqInternal
{
public:
  // Ui::pqSMTKColorByBehavior Actions;
  QAction* ActionArray[NUM_ACTIONS];
  QWidget ActionsOwner;
};

pqSMTKColorByBehavior::pqSMTKColorByBehavior(QObject* parent)
  : Superclass(parent)
{
  m_p = new pqInternal;
  // m_p->Actions.setupUi(&m_p->ActionsOwner);

  if (!s_colorByBar)
  {
    s_colorByBar = this;
  }

  this->setExclusive(false);
}

pqSMTKColorByBehavior::~pqSMTKColorByBehavior()
{
  if (s_colorByBar == this)
  {
    s_colorByBar = nullptr;
  }
}

pqSMTKColorByBehavior* pqSMTKColorByBehavior::instance()
{
  return s_colorByBar;
}

void pqSMTKColorByBehavior::customizeToolBar(QToolBar* tb)
{
  tb->setWindowTitle("SMTK Color Mode");
  auto widget = new pqSMTKColorByWidget(tb);
  widget << pqSetName("colorBy");
  tb->addWidget(widget);
  QObject::connect(&pqActiveObjects::instance(),
    SIGNAL(representationChanged(pqDataRepresentation*)), widget,
    SLOT(setRepresentation(pqDataRepresentation*)));
}

void pqSMTKColorByBehavior::onFilterChanged(QAction* a)
{
  (void)a;
}
