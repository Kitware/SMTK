//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionFilterBehavior.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h" // TODO: remove need for me

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/view/Selection.h"

#include "smtk/io/Logger.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqLiveInsituManager.h"
#include "pqOutputPort.h"
#include "pqPVApplicationCore.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"

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
#include <QWidget>

// Qt generated UI
#include "ui_pqSMTKSelectionFilterBehavior.h"

using namespace smtk;

#define NUM_ACTIONS 8

#define DEBUG_FILTER 0

static pqSMTKSelectionFilterBehavior* s_selectionFilter = nullptr;

class pqSMTKSelectionFilterBehavior::pqInternal
{
public:
  Ui::pqSMTKSelectionFilterBehavior Actions;
  QAction* ActionArray[NUM_ACTIONS];
  QWidget ActionsOwner;
};

// The functions in this namespace are from ParaView's pqStandardViewFrameActionsImplementation.cxx
namespace
{
QAction* findActiveAction(const QString& name)
{
  pqView* activeView = pqActiveObjects::instance().activeView();
  if (
    activeView && activeView->widget() && activeView->widget()->parentWidget() &&
    activeView->widget()->parentWidget()->parentWidget())
  {
    return activeView->widget()->parentWidget()->parentWidget()->findChild<QAction*>(name);
  }
  return nullptr;
}

void triggerAction(const QString& name)
{
  QAction* atcn = findActiveAction(name);
  if (atcn)
  {
    atcn->trigger();
  }
}
} // namespace

pqSMTKSelectionFilterBehavior::pqSMTKSelectionFilterBehavior(QObject* parent)
  : Superclass(parent)
  , m_selection(nullptr)
{
  m_p = new pqInternal;
  m_p->Actions.setupUi(&m_p->ActionsOwner);

  m_p->ActionArray[0] = m_p->Actions.actionSelnAcceptMeshSets;
  m_p->ActionArray[1] = m_p->Actions.actionSelnAcceptModels;
  m_p->ActionArray[2] = m_p->Actions.actionSelnAcceptModelVolumes;
  m_p->ActionArray[3] = m_p->Actions.actionSelnAcceptModelFaces;
  m_p->ActionArray[4] = m_p->Actions.actionSelnAcceptModelEdges;
  m_p->ActionArray[5] = m_p->Actions.actionSelnAcceptModelVertices;
  m_p->ActionArray[6] = m_p->Actions.actionSelnAcceptModelAuxGeoms;
  m_p->ActionArray[7] = m_p->Actions.actionSelnAcceptModelInstances;

  if (!s_selectionFilter)
  {
    s_selectionFilter = this;
  }

  for (int ii = 0; ii < NUM_ACTIONS; ++ii)
  {
    this->addAction(m_p->ActionArray[ii]);
  }
  this->addAction(m_p->Actions.actionStartSMTKSelection);

  // Filters are not all mutually exclusive toggles:
  this->setExclusive(false);
  // By default, all the buttons are off. Set some for the initial filter settings:
  m_p->Actions.actionSelnAcceptModelVertices->setChecked(true);
  m_p->Actions.actionSelnAcceptModelEdges->setChecked(true);
  m_p->Actions.actionSelnAcceptModelFaces->setChecked(true);
  m_p->Actions.actionSelnAcceptModelAuxGeoms->setChecked(true);
  m_p->Actions.actionSelnAcceptModelInstances->setChecked(true);
  // Now force the initial filter to get installed on the selection manager:
  this->onFilterChanged(m_p->Actions.actionSelnAcceptModelVertices);

  QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onFilterChanged(QAction*)));
  QObject::connect(
    m_p->Actions.actionStartSMTKSelection,
    SIGNAL(triggered(bool)),
    this,
    SLOT(startBlockSelectionInActiveView()));

  // Track server connects/disconnects
  auto* rsrcBehavior = pqSMTKBehavior::instance();
  QObject::connect(
    rsrcBehavior,
    SIGNAL(addedManagerOnServer(vtkSMSMTKWrapperProxy*, pqServer*)),
    this,
    SLOT(filterSelectionOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
  QObject::connect(
    rsrcBehavior,
    SIGNAL(removingManagerFromServer(vtkSMSMTKWrapperProxy*, pqServer*)),
    this,
    SLOT(unfilterSelectionOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
}

pqSMTKSelectionFilterBehavior::~pqSMTKSelectionFilterBehavior()
{
  if (s_selectionFilter == this)
  {
    s_selectionFilter = nullptr;
  }
  delete m_p;
}

pqSMTKSelectionFilterBehavior* pqSMTKSelectionFilterBehavior::instance()
{
  return s_selectionFilter;
}

void pqSMTKSelectionFilterBehavior::setSelection(smtk::view::SelectionPtr selnMgr)
{
  m_selection = selnMgr;
}

void pqSMTKSelectionFilterBehavior::onFilterChanged(QAction* a)
{
  // Update all action toggle states to be consistent
  bool acceptMesh = false;
  smtk::model::BitFlags modelFlags = 0;
  if (
    (a == m_p->Actions.actionSelnAcceptMeshSets && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModels && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelVolumes && a->isChecked()))
  { // model-volume and mesh-set selection do not allow other types to be selected at the same time.
    acceptMesh = m_p->Actions.actionSelnAcceptMeshSets->isChecked();
    // Force other model entity buttons off:
    for (int ii = 0; ii < NUM_ACTIONS; ++ii)
    {
      m_p->ActionArray[ii]->setChecked(a == m_p->ActionArray[ii]);
    }
    modelFlags = (m_p->Actions.actionSelnAcceptModels->isChecked() ? smtk::model::MODEL_ENTITY
                                                                   : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelVolumes->isChecked() ? smtk::model::VOLUME
                                                              : smtk::model::NOTHING);
  }
  else if (
    (a == m_p->Actions.actionSelnAcceptModelVertices && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelEdges && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelFaces && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelAuxGeoms && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelInstances && a->isChecked()))
  {
    m_p->Actions.actionSelnAcceptMeshSets->setChecked(false);
    m_p->Actions.actionSelnAcceptModels->setChecked(false);
    m_p->Actions.actionSelnAcceptModelVolumes->setChecked(false);
    acceptMesh = false;
    modelFlags = (m_p->Actions.actionSelnAcceptModelVertices->isChecked() ? smtk::model::VERTEX
                                                                          : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelEdges->isChecked() ? smtk::model::EDGE
                                                            : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelFaces->isChecked() ? smtk::model::FACE
                                                            : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelAuxGeoms->isChecked() ? smtk::model::AUX_GEOM_ENTITY
                                                               : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelInstances->isChecked() ? smtk::model::INSTANCE_ENTITY
                                                                : smtk::model::NOTHING);
  }
  else
  { // Something was turned off, which will not require us to deactivate any other buttons
    acceptMesh = m_p->Actions.actionSelnAcceptMeshSets->isChecked();
    modelFlags = (m_p->Actions.actionSelnAcceptModels->isChecked() ? smtk::model::MODEL_ENTITY
                                                                   : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelVolumes->isChecked() ? smtk::model::VOLUME
                                                              : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelVertices->isChecked() ? smtk::model::VERTEX
                                                               : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelEdges->isChecked() ? smtk::model::EDGE
                                                            : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelFaces->isChecked() ? smtk::model::FACE
                                                            : smtk::model::NOTHING) |
      (m_p->Actions.actionSelnAcceptModelAuxGeoms->isChecked() ? smtk::model::AUX_GEOM_ENTITY
                                                               : smtk::model::NOTHING);
  }
  // Rebuild the selection filter
  m_modelFilterMask = modelFlags;
  m_acceptMeshes = acceptMesh;
  this->installFilter();
}

void pqSMTKSelectionFilterBehavior::startBlockSelectionInActiveView()
{
  triggerAction("actionSelectBlock");
}

void pqSMTKSelectionFilterBehavior::filterSelectionOnServer(
  vtkSMSMTKWrapperProxy* mgr,
  pqServer* server)
{
  (void)server;
  if (!mgr)
  {
#if !defined(NDEBUG) && DEBUG_FILTER
    std::cout << "  filterSelectionOnServer: no mgr\n";
#endif
    return;
  }
  if (
    pqLiveInsituManager::isInsituServer(server) ||
    pqLiveInsituManager::instance()->isDisplayServer(server))
  {
#if !defined(NDEBUG) && DEBUG_FILTER
    std::cout << "  server is catalyst connection... ignoring.\n";
#endif
    return;
  }
  //auto seln = mgr->GetSelection();
  auto seln = vtkSMTKWrapper::SafeDownCast(mgr->GetClientSideObject())
                ->GetSelection(); // TODO: Only works on built-in server
  if (!seln)
  {
#if !defined(NDEBUG) && DEBUG_FILTER
    std::cout << "  filterSelectionOnServer: no seln\n";
#endif
    return;
  }

  if (m_selection)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "pqSMTKSelectionFilterBehavior can currently only manage a single ParaView server. "
      "No more updates will be provided for "
        << m_selection << " in favor of " << seln);
  }

#if !defined(NDEBUG) && DEBUG_FILTER
  std::cout << "  filter on s " << seln << " m_ " << m_selection << "\n";
#endif
  m_selection = seln;
  this->installFilter();
}

void pqSMTKSelectionFilterBehavior::unfilterSelectionOnServer(
  vtkSMSMTKWrapperProxy* mgr,
  pqServer* server)
{
  (void)server;
#if !defined(NDEBUG) && DEBUG_FILTER
  std::cout << "  unfilterSelectionOnServer: " << server << "\n\n";
#endif

  // Drop filtering on existing server.
  if (m_selection)
  {
    m_selection->setFilter(nullptr);
    m_selection = nullptr;
  }

  if (!mgr)
  {
    return;
  }
  auto seln = mgr->GetSelection();
  if (!seln && !m_selection)
  {
    return;
  }

  if (!seln)
  {
    seln = m_selection;
  }

  seln->setFilter(nullptr);
}

void pqSMTKSelectionFilterBehavior::installFilter()
{
#if !defined(NDEBUG) && DEBUG_FILTER
  std::cout << "    Updating filter on " << m_selection << " " << m_modelFilterMask << "\n";
#endif
  if (!m_selection)
  {
    return;
  }

  bool acceptMeshes = m_acceptMeshes;
  smtk::model::BitFlags modelFlags = m_modelFilterMask;

  m_selection->setFilter([acceptMeshes, modelFlags](
                           smtk::resource::PersistentObjectPtr comp,
                           int value,
                           smtk::view::Selection::SelectionMap& suggestions) {
    if (acceptMeshes)
    {
      auto mesh = std::dynamic_pointer_cast<smtk::mesh::Component>(comp);
      if (mesh != nullptr)
      {
        return true;
      }
    }
    if (modelFlags)
    {
      auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
      if (!modelEnt)
      {
        return false;
      }
      auto entBits = modelEnt->entityFlags();
      // Is the entity of an acceptable type?
      if ((modelFlags & smtk::model::MODEL_ENTITY) == smtk::model::MODEL_ENTITY)
      {
        smtk::model::EntityRef eref(modelEnt->modelResource(), modelEnt->id());
        smtk::model::Model model = eref.owningModel();
        smtk::model::EntityPtr suggestion;
        if (model.isValid(&suggestion))
        {
          suggestions.insert(std::make_pair(suggestion, value));
        }
      }
      else if ((modelFlags & smtk::model::VOLUME) == smtk::model::VOLUME)
      {
        smtk::model::CellEntity cell(modelEnt->modelResource(), modelEnt->id());
        smtk::model::Volumes vv;
        if (cell.isValid())
        {
          switch (cell.dimension())
          {
            case 0:
            {
              smtk::model::Edges edges = cell.as<smtk::model::Vertex>().edges();
              for (const auto& edge : edges)
              {
                smtk::model::Faces faces = edge.faces();
                for (const auto& face : faces)
                {
                  smtk::model::Volumes tmp = face.volumes();
                  vv.insert(vv.end(), tmp.begin(), tmp.end());
                }
              }
            }
            break;
            case 1:
            {
              smtk::model::Faces faces = cell.as<smtk::model::Edge>().faces();
              for (const auto& face : faces)
              {
                smtk::model::Volumes tmp = face.volumes();
                vv.insert(vv.end(), tmp.begin(), tmp.end());
              }
            }
            break;
            case 2:
              vv = cell.as<smtk::model::Face>().volumes();
              break;
            case 3:
              vv.push_back(cell.as<smtk::model::Volume>());
              break;
            default:
              break;
          }
        }
        smtk::model::EntityPtr suggestion;
        for (const auto& vc : vv)
        {
          if (vc.isValid(&suggestion))
          {
            suggestions.insert(std::make_pair(suggestion, value));
          }
        }
      }
      else if ((entBits & smtk::model::ENTITY_MASK) & (modelFlags & smtk::model::ENTITY_MASK))
      {
        // Ensure the dimension is acceptable, too:
        return (entBits & smtk::model::AUX_GEOM_ENTITY) ||
          (entBits & smtk::model::INSTANCE_ENTITY) ||
          ((entBits & smtk::model::ANY_DIMENSION) & (modelFlags & smtk::model::ANY_DIMENSION));
      }
      else
      {
        return false;
      }
    }
    return true; // If no filtering is requested, allow everything
  });
}
