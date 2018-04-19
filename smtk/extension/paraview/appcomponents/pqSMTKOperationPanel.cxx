//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/operation/Operation.h"

#include "smtk/view/AvailableOperations.h"
#include "smtk/view/Selection.h"
#include "smtk/view/View.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/workflow/OperationFilterSort.h"
#include "smtk/workflow/json/jsonOperationFilterSort.h"

#include "smtk/io/Logger.h"

#include "pqActiveObjects.h"
#include "pqPipelineSource.h"

#include "ui_pqSMTKOperationPanel.h"

#include <QListWidget>
#include <QPointer>
#include <QVBoxLayout>
#include <QWidget>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class pqSMTKOperationPanel::Internal : public Ui::pqSMTKOperationPanel
{
public:
  Internal() {}

  ~Internal() {}

  void setup(::pqSMTKOperationPanel* panel)
  {
    QWidget* ww = new QWidget(panel);
    panel->setObjectName("OperationPanel");
    panel->setWindowTitle("Operations");
    panel->setWidget(ww);
    this->setupUi(ww);
    this->OperationEditor->setLayout(new QVBoxLayout(this->OperationEditor));
    this->OperationList->setOperationSource(panel->availableOperations());

    QObject::connect(
      this->UseSelection, SIGNAL(toggled(bool)), panel, SLOT(toggleFilterBySelection(bool)));

    QObject::connect(OperationList->listWidget(), SIGNAL(itemClicked(QListWidgetItem*)), panel,
      SLOT(operationListClicked(QListWidgetItem*)));
    QObject::connect(OperationList->listWidget(), SIGNAL(itemDoubleClicked(QListWidgetItem*)),
      panel, SLOT(operationListDoubleClicked(QListWidgetItem*)));
    QObject::connect(OperationList->listWidget(), SIGNAL(itemActivated(QListWidgetItem*)), panel,
      SLOT(operationListActivated(QListWidgetItem*)));
    QObject::connect(OperationList->listWidget(),
      SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), panel,
      SLOT(operationListCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));
  }
};

pqSMTKOperationPanel::pqSMTKOperationPanel(QWidget* parent)
  : Superclass(parent)
  , m_p(nullptr)
  , m_editing(nullptr)
  , m_attrUIMgr(nullptr)
{
  // This must come before m_p is created, since the m_p->OperationList widget will reference it:
  auto opFilterSort = smtk::workflow::OperationFilterSort::create();
  m_availableOperations = smtk::view::AvailableOperations::create();
  m_availableOperations->setWorkflowFilter(opFilterSort);

  m_p = new Internal;
  m_p->setup(this);

  auto behavior = pqSMTKBehavior::instance();
  QObject::connect(behavior, SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(observeWrapper(pqSMTKWrapper*, pqServer*)));
  QObject::connect(behavior, SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(unobserveWrapper(pqSMTKWrapper*, pqServer*)));
  // Initialize with current wrapper(s), if any:
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeWrapper(wrapper, server);
    return false; // terminate early
  });
}

pqSMTKOperationPanel::~pqSMTKOperationPanel()
{
  delete m_attrUIMgr;
  delete m_p;
}

void pqSMTKOperationPanel::observeWrapper(pqSMTKWrapper* wrapper, pqServer*)
{
  if (wrapper)
  {
    // TODO: This is a placeholder for creating a real workflow
    // std::string readFilePath("/stage/source/cmb/6/thirdparty/smtk/data");
    // std::string readFilePath("/Users/tjcorona/Development/Software/cmb/cmb/source/thirdparty/smtk/data");
    // std::string jsonFile("/workflow/filterList.json");
    // std::ifstream file((readFilePath + jsonFile).c_str());
    // std::string data((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    // json j = json::parse(data);
    smtk::workflow::OperationFilterSortPtr opFilterSort;
    // smtk::workflow::from_json(j, opFilterSort, wrapper->smtkOperationManager());

    m_availableOperations->setSelection(wrapper->smtkSelection());
    m_availableOperations->setOperationManager(wrapper->smtkOperationManager());
    m_availableOperations->setWorkflowFilter(opFilterSort);
  }
  else
  {
    this->m_availableOperations->setSelection(nullptr);
    this->m_availableOperations->setOperationManager(nullptr);
  }
}

void pqSMTKOperationPanel::unobserveWrapper(pqSMTKWrapper* wrapper, pqServer*)
{
  if (wrapper) // TBD: compare wrapper's managers to what we're currently observing?
  {
    this->m_availableOperations->setSelection(nullptr);
    this->m_availableOperations->setOperationManager(nullptr);
  }
}

bool pqSMTKOperationPanel::editOperation(smtk::operation::Operation::Index index)
{
  if (!m_availableOperations || !m_availableOperations->operationManager())
  {
    return false;
  }

  auto op = m_availableOperations->operationManager()->create(index);
  if (!op)
  {
    return false;
  }

  return this->editOperation(op);
}

bool pqSMTKOperationPanel::editOperation(smtk::operation::OperationPtr op)
{
  bool didDisplay = false;

  // Can't display nuthin'.
  if (!op)
  {
    return didDisplay;
  }

  // Don't re-display what is already displayed.
  if (m_attrUIMgr && m_attrUIMgr->operation() == op)
  {
    return didDisplay;
  }

  // Let go of what we are already displaying.
  if (m_rsrc)
  {
    auto rsrcMgr = m_rsrc->manager();
    if (rsrcMgr && m_observer >= 0)
    {
      rsrcMgr->unobserve(m_observer);
    }
    m_editing = nullptr;
  }

  // Hold on to what we're about to display.
  m_editing = op;
  m_rsrc = op->specification();

  // Destroy previous UI widgets.
  if (m_attrUIMgr)
  {
    delete m_attrUIMgr;
    while (QWidget* w = m_p->OperationEditor->findChild<QWidget*>())
    {
      delete w;
    }
  }

  // Create a new UI.
  m_attrUIMgr = new smtk::extension::qtUIManager(m_editing);
  if (!m_attrUIMgr->operationManager())
  {
    m_attrUIMgr->setOperationManager(m_availableOperations->operationManager());
  }

  smtk::view::ViewPtr view = m_attrUIMgr->findOrCreateOperationView();
  auto baseView = m_attrUIMgr->setSMTKView(view, m_p->OperationEditor);
  didDisplay = baseView ? true : false;

  auto rsrcMgr = m_rsrc->manager();
  if (rsrcMgr)
  {
    // If the operation's specification is destroyed, then
    // get rid of the UI.
    m_observer = rsrcMgr->observe(
      [this, rsrcMgr](smtk::resource::Event evnt, smtk::resource::Resource::Ptr attrRsrc) {
        if (evnt == smtk::resource::Event::RESOURCE_REMOVED && attrRsrc == m_rsrc)
        {
          // The application is removing the attribute resource we are viewing.
          // Clear out the panel and unobserve the manager.
          rsrcMgr->unobserve(m_observer);
          delete m_attrUIMgr;
          while (QWidget* w = m_p->OperationEditor->findChild<QWidget*>())
          {
            delete w;
          }
          m_attrUIMgr = nullptr;
          m_rsrc = nullptr;
        }
      });
  }
  return didDisplay;
}

void pqSMTKOperationPanel::runOperation()
{
  this->runOperation(m_editing);
}

void pqSMTKOperationPanel::runOperation(smtk::operation::OperationPtr operation)
{
  if (!operation)
  {
    return;
  }
  operation->operate();
}

void pqSMTKOperationPanel::cancelEditing()
{
  // TODO
  m_p->OperationEditor->hide();
}

void pqSMTKOperationPanel::toggleFilterBySelection(bool showFiltered)
{
  m_availableOperations->setUseSelection(showFiltered);
}

void pqSMTKOperationPanel::operationListClicked(QListWidgetItem* item)
{
  if (!item)
  {
    return;
  }

  QVariant opIdx = item->data(Qt::UserRole + 47);
  if (opIdx.isValid())
  {
    const auto& allMeta =
      m_availableOperations->operationManager()->metadata().get<smtk::operation::IndexTag>();
    auto index = qvariant_cast<smtk::operation::Operation::Index>(opIdx);
    auto opMeta = allMeta.find(index);
    if (opMeta != allMeta.end())
    {
      this->displayDocumentation(index);
    }
  }
}

void pqSMTKOperationPanel::operationListDoubleClicked(QListWidgetItem* item)
{
  if (!item)
  {
    return;
  }

  QVariant opIdx = item->data(Qt::UserRole + 47);
  if (opIdx.isValid())
  {
    auto index = qvariant_cast<smtk::operation::Operation::Index>(opIdx);
    auto opInstance = m_availableOperations->operationManager()->create(index);
    auto seln = m_availableOperations->selection();
    const auto& smap = seln->currentSelection();
    auto params = opInstance->parameters();
    for (auto entry : smap)
    {
      if ((entry.second & 0x01) ==
        0x01) // FIXME: properly select entities from the map based on a specific bit flag
      {
        params->associate(entry.first);
      }
    }
    if (this->editOperation(opInstance))
    {
      //m_p->OperationList->hide();
    }
  }
}

void pqSMTKOperationPanel::operationListActivated(QListWidgetItem* item)
{
  (void)item;
}

void pqSMTKOperationPanel::operationListCurrentItemChanged(
  QListWidgetItem* item, QListWidgetItem* prev)
{
  (void)prev;
  if (item)
  {
    m_p->OperationDocs->show();
  }
  else
  {
    m_p->OperationDocs->hide();
  }
}

void pqSMTKOperationPanel::displayDocumentation(const smtk::operation::Operation::Index& index)
{
  (void)index;
}
