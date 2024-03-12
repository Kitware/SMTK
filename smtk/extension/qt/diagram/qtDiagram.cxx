//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagram.h"

#include "smtk/extension/qt/diagram/PanelConfiguration_cpp.h"
#include "smtk/extension/qt/diagram/qtBaseArc.h"
#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"
#include "smtk/extension/qt/diagram/qtDiagramLegend.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramView.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtTaskArc.h"

#include "smtk/view/Selection.h"

#include "smtk/common/Managers.h"
#include "smtk/common/json/jsonUUID.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtManager.h"

// nodes
#include "smtk/extension/qt/diagram/qtDefaultTaskNode.h"
#include "smtk/extension/qt/diagram/qtResourceNode.h"

// modes
#include "smtk/extension/qt/diagram/qtConnectMode.h"
#include "smtk/extension/qt/diagram/qtDisconnectMode.h"
#include "smtk/extension/qt/diagram/qtPanMode.h"
#include "smtk/extension/qt/diagram/qtSelectMode.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/task/Active.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/ResultOps.h"
#include "smtk/operation/groups/ArcCreator.h"

#include "smtk/resource/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/io/Logger.h"

#include "smtk/Regex.h"

#include "nlohmann/json.hpp"

#include <QAction>
#include <QActionGroup>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QPalette>
#include <QPointF>
#include <QScrollArea>
#include <QSizeGrip>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <QWidgetAction>

// Uncomment to get debug printouts from workflow events.
// #define SMTK_DBG_WORKFLOWS 1

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

class qtDiagram::Internal
{
public:
  Internal(qtDiagram* self, const smtk::view::Information& info)
    : m_configuration(info.configuration())
    , m_self(self)
    , m_scene(new qtDiagramScene(m_self))
    , m_widget(new QWidget)
    // m_widget contains a QGridLayout with two overlapping widgets:
    , m_sidebarOuterLayout(new QGridLayout(m_widget))
    , m_sidebarOuter(new QFrame)
    , m_view(new qtDiagramView(m_scene, m_self))
    // m_sidebarOuter contains a QGridLayout with two overlapping widgets:
    , m_sidebarMiddleLayout(new QVBoxLayout(m_sidebarOuter))
    , m_sidebarSizer(new QSizeGrip(m_sidebarOuter))
    , m_sidebarMiddle(new QScrollArea)
    // m_sidebarMiddle contains a viewport holding the actual sidebar contents:
    , m_sidebarInner(new QWidget)
    , m_sidebarInnerLayout(new QVBoxLayout(m_sidebarInner))
    // m_sidebarInner contains widgets from generators plus the legend:
    , m_legend(new qtDiagramLegend("Legend", m_self))
  {
    // If this view is inside a dock-widget, take over the dock's title-bar.
    // Otherwise, put the dock inside m_sidebarOuterLayout above m_view.
    m_dock = nullptr;
    QObject* dp = info.get<QWidget*>();
    while (dp && !m_dock)
    {
      m_dock = qobject_cast<QDockWidget*>(dp);
      dp = dp->parent();
    }
    const int gridRow = m_dock ? 0 : 1;
    m_toolbar = new QToolBar(m_dock ? static_cast<QPointer<QWidget>>(m_dock) : m_widget);
    m_toolbar->setObjectName("DiagramToolbar");
    m_toolbar->setIconSize(QSize(16, 16));

    m_scene->setObjectName("DiagramScene");
    m_widget->setObjectName("OuterDiagramGrid");
    m_widget->setLayout(m_sidebarOuterLayout);
    m_self->Widget = m_widget;

    m_sidebarOuterLayout->setObjectName("SidebarOuterLayout");
    m_sidebarOuterLayout->setSpacing(0);
    if (m_dock)
    {
      m_dock->setTitleBarWidget(m_toolbar);
    }
    else
    {
      m_sidebarOuterLayout->addWidget(m_toolbar, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    }
    m_sidebarOuterLayout->addWidget(m_view, gridRow, 0, Qt::AlignLeft | Qt::AlignTop);
    m_sidebarOuterLayout->addWidget(m_sidebarOuter, gridRow, 0, Qt::AlignLeft | Qt::AlignTop);
    m_sidebarOuter->setObjectName("SidebarOuter");
    // Mark the sidebar "outer" widget as a subwindow so it will
    // be resized by the QSizeGrip contained inside in the "middle" layout.
    m_sidebarOuter->setWindowFlags(m_sidebarOuter->windowFlags() | Qt::SubWindow);
    m_sidebarOuter->setMinimumWidth(200);
    m_sidebarOuter->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    m_sidebarOuter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_sidebarOuter->setAutoFillBackground(true);
    m_sidebarOuter->setLayout(m_sidebarMiddleLayout);
    m_view->setObjectName("DiagramView");

    auto* sizerLayout = new QHBoxLayout;
    m_sidebarMiddleLayout->setObjectName("SidebarMiddleLayout");
    m_sidebarMiddleLayout->setMargin(0);
    m_sidebarMiddleLayout->addWidget(m_sidebarMiddle);
    m_sidebarMiddleLayout->addLayout(sizerLayout);
    m_sidebarMiddle->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    m_sidebarSizer->setObjectName("SidebarResizer");
    sizerLayout->addStretch(1);
    sizerLayout->setObjectName("RightJustifySizeGrip");
    sizerLayout->addWidget(m_sidebarSizer);
    m_sidebarMiddle->setObjectName("SidebarMiddle");
    m_sidebarMiddle->setWidget(m_sidebarInner);
    m_sidebarMiddle->setWidgetResizable(true);
    m_sidebarMiddle->setAutoFillBackground(true);

    m_sidebarInner->setObjectName("SidebarInner");
    m_sidebarInnerLayout->setObjectName("SidebarInnerLayout");
    m_sidebarInner->setLayout(m_sidebarInnerLayout);

    // Add the legend (created in the constructor) to the layout
    // Make it extra-stretchy so that as much of the legend entries
    // will be shown as feasible (given generators may also insert
    // widgets with stretch factors).
    m_sidebarInnerLayout->addWidget(m_legend);
    m_sidebarInnerLayout->setStretchFactor(m_legend, 4);
    bool showLegend = true;
    if (m_configuration)
    {
      m_configuration->details().attributeAsBool("Legend", showLegend);
    }
    m_legend->setVisible(showLegend);

    std::string panelTitle = "Workflow Diagram";
    if (m_configuration)
    {
      m_configuration->details().attribute("Title", panelTitle);
      m_scene->setConfiguration(new qtDiagramViewConfiguration(*m_configuration));
    }
    const auto& managers = info.get<std::shared_ptr<smtk::common::Managers>>();
    smtk::view::Selection::Ptr selection;
    if (managers)
    {
      m_managers = managers;
      m_operationManager = managers->get<smtk::operation::Manager::Ptr>();

      selection = managers->get<smtk::view::Selection::Ptr>();
      if (selection)
      {
        std::ostringstream sourceName;
        sourceName << "qtDiagram " << m_self;
        m_selectionSource = sourceName.str();
        if (!selection->registerSelectionSource(m_selectionSource))
        {
          m_selectionSource.clear();
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not register selection source \"" << m_selectionSource << "\".");
        }
        m_selectionValue = selection->findOrCreateLabeledValue("selected");
      }
    }

    // Add a control to toggle the sidebar.
    m_toggleSidebar = new QAction(QString::fromStdString("◧"), m_self);
    m_toggleSidebar->setToolTip("Toggle side-bar visibility");
    m_toggleSidebar->setCheckable(true);
    m_toggleSidebar->setChecked(true);
    m_toggleSidebar->setObjectName("ToggleSidebarVisibility");
    m_toolbar->addAction(m_toggleSidebar);
    QObject::connect(m_toggleSidebar, &QAction::toggled, m_self, &qtDiagram::toggleSidebar);

    if (m_dock)
    {
      auto* lbl = new QLabel(panelTitle.c_str());
      lbl->setObjectName("DiagramTitle");
      lbl->setTextFormat(Qt::RichText);
      m_toolbar->addWidget(lbl);
    }
    auto* spacer = new QFrame;
    spacer->setObjectName("DiagramToolbarSpacer");
    spacer->setFrameShape(QFrame::NoFrame);
    spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    spacer->setMinimumSize(20, 1);
    m_toolbar->addWidget(spacer);
    m_taskMode = new QActionGroup(m_toolbar);
    m_taskMode->setObjectName("DiagramModeGroup");
    QObject::connect(m_taskMode, &QActionGroup::triggered, self, &qtDiagram::modeChangeRequested);
  }

  ~Internal()
  {
    if (m_managers && m_managers->contains<smtk::view::Selection::Ptr>())
    {
      auto selection = m_managers->get<smtk::view::Selection::Ptr>();
      if (!m_selectionSource.empty())
      {
        if (!selection->unregisterSelectionSource(m_selectionSource))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not unregister selection source \"" << m_selectionSource << "\".");
        }
      }
    }
  }

  // Called after construction of the qtDiagram is complete to finalize
  // initialization of the UI and internal structures.
  void initialize()
  {
    std::shared_ptr<smtk::extension::qtManager> qtMgr;
    if (m_managers && m_managers->contains<smtk::extension::qtManager::Ptr>())
    {
      qtMgr = m_managers->get<smtk::extension::qtManager::Ptr>();
    }
    if (qtMgr)
    {
      qtDiagramViewMode* defaultMode = nullptr;
      // Create generators and modes as listed in the configuration.
      for (const auto& child : m_configuration->details().children())
      {
        if (child.name() == "Diagram")
        {
          std::string generatorType;
          std::string generatorName;
          if (child.attribute("Type", generatorType) && child.attribute("Name", generatorName))
          {
            // Use a factory to create generator
            auto generator = qtMgr->diagramGeneratorFactory().makeFromName(
              generatorType, m_self->information(), child, m_self);
            // For debugging:
            // std::cout << "Add generator " << generatorType << " as " << generatorName << " " << generator << "\n";
            if (generator)
            {
              m_generators[generatorName] = generator;
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                "Could not create a diagram generator of type " << generatorType << " for "
                                                                << generatorName << ".");
            }
          }
          else
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Diagram generator configurations must specify a Type and a Name.");
          }
        }
        else if (child.name() == "Mode")
        {
          // Construct a view mode:
          //   + creates an action (fetchable via modeAction()) from m_taskMode (the QActionGroup)
          //   + installs the modeAction() into the QToolBar, i.e.: m_toolbar->addAction(viewMode->modeAction());
          //   + may add more widgets/actions to QToolBar (e.g., qtConnectMode).
          std::string modeType;
          if (child.attribute("Type", modeType))
          {
            auto viewMode = qtMgr->diagramViewModeFactory().makeFromName(
              modeType, m_self, m_view, m_toolbar, m_taskMode);
            // For debugging:
            // std::cout << "Add view mode " << modeType << " " << viewMode << "\n";
            if (viewMode)
            {
              m_modeMap[viewMode->modeAction()->objectName().toStdString()] = viewMode;
              if (child.attributeAsBool("Default") || !defaultMode)
              {
                defaultMode = viewMode.get();
              }
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                "Could not create a view mode of type " << modeType << ".");
            }
          }
          else
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(), "View mode configurations must specify a Type.");
          }
        }
      }

      m_taskMode->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
      if (defaultMode)
      {
        // Force the initial mode to be the one marked as "Default" (if one is):
        defaultMode->modeAction()->setChecked(true);
        defaultMode->enterMode();
        m_mode = defaultMode->modeAction()->objectName().toStdString();
        m_defaultModeName = m_mode;
      }
    }

    if (m_operationManager)
    {
      m_onKey = m_operationManager->observers().insert(
        [this](
          const smtk::operation::Operation& op,
          smtk::operation::EventType event,
          smtk::operation::Operation::Result result) {
          if (event == smtk::operation::EventType::DID_OPERATE)
          {
            this->updateFromOperation(op, result);
          }
          return 0;
        },
        "Task editor.");
    }

    // Now that generators have been initialized, add our
    // selection observer (which may immediately overwrite
    // the Qt scene's selection with the SMTK selection).
    if (!m_selectionSource.empty())
    {
      auto selection = m_managers->get<smtk::view::Selection::Ptr>();
      if (selection)
      {
        QPointer<qtDiagram> selfOrNull(m_self);
        m_selectionObserver = selection->observers().insert(
          [this, selfOrNull](const std::string& changeSource, smtk::view::Selection::Ptr) {
            // Do nothing if our parent qtDiagram object has been destroyed.
            if (!selfOrNull)
            {
              return;
            }
            // Do nothing if the source of the change is this widget.
            if (changeSource == m_selectionSource)
            {
              return;
            }
            // It is safe to overwrite the Qt selection with SMTK's:
            m_self->updateQtSelection();
          },
          /* priority */ 0,
          /* initialize */ true,
          /* description */ m_selectionSource);
      }
    }
  }

  // Returns true if UI configuration was modified
  bool computeNodeLayout()
  {
    if (m_nodeIndex.empty() || (!m_layoutMap.empty()))
    {
      return false;
    }

    // If geometry not configured, call the scene's method to layout nodes
    std::unordered_set<qtBaseNode*> nodes;
    std::unordered_set<qtBaseArc*> arcs;
    for (const auto& entry : m_nodeIndex)
    {
      nodes.insert(entry.second);
    }
    for (const auto& entry : m_arcIndex)
    {
      for (const auto& arcSetMap : entry.second)
      {
        for (const auto& arcSet : arcSetMap.second)
        {
          arcs.insert(arcSet.second.begin(), arcSet.second.end());
        }
      }
    }
    return (m_scene->computeLayout(nodes, arcs) == 1);
  }

  void clear()
  {
    m_scene->clear();
    m_nodeIndex.clear();
    m_arcIndex.clear();

    // Inform all the modes the scene has been cleared so they
    // can add new items if needed.
    for (const auto& entry : m_modeMap)
    {
      entry.second->sceneCleared();
    }
  }

  bool configure(const nlohmann::json& data)
  {
    bool ok = true;
    auto it = data.find("layout");
    if (it != data.end())
    {
      m_layoutMap = it->get<std::unordered_map<smtk::common::UUID, std::array<double, 2>>>();
    }
    // Now give each diagram generator a chance to fetch application state:
    for (const auto& entry : m_generators)
    {
      ok &= entry.second->configure(data);
    }
    return ok;
  }

  nlohmann::json configuration() const
  {
    nlohmann::json config;
    // Give generators a chance to insert data.
    for (const auto& entry : m_generators)
    {
      entry.second->addConfiguration(config);
    }
    // Now update and insert the diagram's node layout.
    m_layoutMap.clear();
    for (const auto& entry : m_nodeIndex)
    {
      auto* node = entry.second;
      auto nodeId = entry.first;
      auto qpoint = node->pos();
      m_layoutMap[nodeId] = { { qpoint.x(), qpoint.y() } };
    }
    config["layout"] = m_layoutMap;
    return config;
  }

  void updateFromOperation(
    const smtk::operation::Operation& op,
    const smtk::operation::Operation::Result& result)
  {
    (void)op;
    std::unordered_set<smtk::resource::PersistentObject*> created;
    std::unordered_set<smtk::resource::PersistentObject*> modified;
    std::unordered_set<smtk::resource::PersistentObject*> expunged;

    std::unordered_set<smtk::resource::PersistentObject*>* destination = nullptr;

    smtk::resource::Component::Visitor vv =
      [this, &destination](const smtk::resource::Component::Ptr& comp) {
        destination->insert(comp.get());
      };

    // Populate the created objects (both resources and components)
    destination = &created;
    for (const auto& rsrc : smtk::operation::createdResourcesOfResult(result))
    {
      // Resource was added
      created.insert(rsrc.get());
      rsrc->visit(vv);
    }
    // Now consider components reported by the operation
    auto createdItem = result->findComponent("created");
    for (const auto& value : *createdItem)
    {
      vv(std::static_pointer_cast<smtk::resource::Component>(value));
    }

    // Populate the expunged objects (both resources and components)
    destination = &expunged;
    for (const auto& rsrc : smtk::operation::expungedResourcesOfResult(result))
    {
      expunged.insert(rsrc.get());
      rsrc->visit(vv);
    }
    auto expungedItem = result->findComponent("expunged");
    for (const auto& value : *expungedItem)
    {
      vv(std::static_pointer_cast<smtk::resource::Component>(value));
    }

    // Populate the modified objects (only components at this point)
    destination = &modified;
    auto modifiedItem = result->findComponent("modified");
    for (const auto& value : *modifiedItem)
    {
      vv(std::static_pointer_cast<smtk::resource::Component>(value));
    }

    // Now iterate over diagram generators to process the objects.
    this->resetViewHints();
    for (const auto& entry : m_generators)
    {
      entry.second->updateScene(created, modified, expunged, op, result);
    }
    this->applyViewHints();
  }

  void resetViewHints() { m_inclusions.clear(); }

  void includeInView(const QRectF& inclusion, int priority)
  {
    (void)priority; // Ignore for now.
    m_inclusions.push_back(inclusion);
  }

  void applyViewHints()
  {
    // Grab current viewport rectangle (adjusted by a margin since fitInView seems to apply one):
    int fw = m_view->frameWidth();
    QRectF finalRect = m_view->mapToScene(m_view->viewport()->rect())
                         .boundingRect()
                         .adjusted(2 * fw, 2 * fw, -2 * fw, -2 * fw);
    // Now union it with the hints provided by all the generators:
    for (const auto& rect : this->m_inclusions)
    {
      finalRect = finalRect.united(rect);
    }
    // Tell Qt to scale and shift to fit the result into the viewport:
    m_view->fitInView(finalRect, Qt::KeepAspectRatio);
  }

  const smtk::view::Configuration* m_configuration;
  qtDiagram* m_self{ nullptr };
  qtDiagramScene* m_scene{ nullptr };
  QPointer<QWidget> m_widget;

  QPointer<QGridLayout> m_sidebarOuterLayout;
  QPointer<QFrame> m_sidebarOuter;
  qtDiagramView* m_view{ nullptr };

  QPointer<QVBoxLayout> m_sidebarMiddleLayout;
  QPointer<QSizeGrip> m_sidebarSizer;
  QPointer<QScrollArea> m_sidebarMiddle;

  QPointer<QWidget> m_sidebarInner;
  QPointer<QVBoxLayout> m_sidebarInnerLayout; // Holds widgets added by modes/generators.
  qtDiagramLegend* m_legend{ nullptr };

  QToolBar* m_toolbar{ nullptr };
  QActionGroup* m_taskMode{ nullptr };

  // Observer that keeps other objects up to date.
  smtk::operation::Observers::Key m_onKey;

  // Nodes mapped from their source UUID:
  std::unordered_map<smtk::common::UUID, qtBaseNode*> m_nodeIndex;
  // Arcs grouped by their predecessor, then successor node, then arc type:
  std::unordered_map<
    qtBaseNode*,
    std::unordered_map<
      qtBaseNode*,
      std::unordered_map<smtk::string::Token, std::unordered_set<qtBaseArc*>>>>
    m_arcIndex;
  // Successor node reverse-lookup map. Given a successor, return all arc-predecessors.
  std::unordered_map<qtBaseNode*, std::unordered_set<qtBaseNode*>> m_arcReverseIndex;

  // Configuration that an operation (read, emplace-worklet, etc.) may provide for layout.
  mutable std::unordered_map<smtk::common::UUID, std::array<double, 2>> m_layoutMap;

  // The set of all diagram generators:
  std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramGenerator>> m_generators;

  // The current interaction mode for the editor.
  smtk::string::Token m_mode;
  // The default mode for the editor (the first mode added if no default is provided).
  smtk::string::Token m_defaultModeName;
  // The set of all interaction modes:
  std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramViewMode>> m_modeMap;

  // The operation manager obtained from the view information.
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  // The common::Managers object obtained from the view information.
  std::shared_ptr<smtk::common::Managers> m_managers;

  // Should task-nodes be enabled?
  bool m_nodesEnabled{ true };
  // Should task-nodes be selectable?
  bool m_nodeSelectionEnabled{ true };
  // Should arc selection be enabled (and node selection be disabled).
  bool m_arcSelectionEnabled{ false };

  /// Rectangles hinted by generators for inclusion into the viewport
  std::vector<QRectF> m_inclusions;

  QPointer<QDockWidget> m_dock;
  QPointer<QAction> m_toggleSidebar;

  std::string m_selectionSource;
  int m_selectionValue{ -1 };
  smtk::view::SelectionObservers::Key m_selectionObserver;
  bool m_selectionUpdatingFromSMTK{ false }; // True only inside updateQtSelection().
  QRect m_lastSidebarRect;
};

qtDiagram::qtDiagram(const smtk::view::Information& info)
  : qtBaseView(info)
  , m_p(new Internal(this, info))
{
  this->setObjectName("qtDiagram");
  // When the Qt selection changes, update the SMTK selection.
  QObject::connect(
    m_p->m_scene, &QGraphicsScene::selectionChanged, this, &qtDiagram::updateSMTKSelection);
  // When the SMTK selection changes, we also update the Qt selection; see m_p->initialize() for details.
  m_p->initialize();
}

qtDiagram::~qtDiagram()
{
  // Delete modes explicitly here so any installed event filters are removed
  while (!m_p->m_modeMap.empty())
  {
    auto it = m_p->m_modeMap.begin();
    m_p->m_modeMap.erase(it); // releases the shared pointer
  }
  m_p->m_self = nullptr;
}

qtBaseView* qtDiagram::createViewWidget(const smtk::view::Information& info)
{
  qtDiagram* editor = new qtDiagram(info);
  // editor->buildUI();
  return editor;
}

void qtDiagram::requestModeChange(smtk::string::Token mode)
{
  if (m_p->m_mode == mode)
  {
    return;
  }
  auto it = m_p->m_modeMap.find(mode);
  if (it == m_p->m_modeMap.end())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Unknown mode \"" << mode.data() << "\" requested. Ignoring.");
    return;
  }
  it->second->modeAction()->trigger();
}

void qtDiagram::toggleSidebar(bool showSidebar)
{
  if (showSidebar)
  {
    m_p->m_sidebarOuter->show();
    if (m_p->m_lastSidebarRect.isValid())
    {
      m_p->m_sidebarOuter->setGeometry(m_p->m_lastSidebarRect);
    }
  }
  else
  {
    m_p->m_lastSidebarRect = m_p->m_sidebarOuter->geometry();
    m_p->m_sidebarOuter->hide();
  }
}

bool qtDiagram::updateSMTKSelection()
{
  bool didChange = false;
  if (m_p->m_selectionUpdatingFromSMTK)
  {
    return didChange;
  }

  auto mgrs = this->managers();
  if (!mgrs)
  {
    return didChange;
  }
  if (!mgrs->contains<smtk::view::Selection::Ptr>())
  {
    return didChange;
  }
  auto smtkSelection = mgrs->get<smtk::view::Selection::Ptr>();
  if (!smtkSelection)
  {
    return didChange;
  }

  // TODO: The dynamic casts in this method require the base diagram class
  //       to depend on nodes/arcs defined by particular generators. This
  //       ought not be. But having every generator respond to selection
  //       changes seems tedious…
  std::set<smtk::resource::PersistentObject::Ptr> selected;
  std::size_t nn = 0;
  if (m_p->m_nodesEnabled && m_p->m_nodeSelectionEnabled)
  {
    for (const auto* item : m_p->m_scene->selectedItems())
    {
      if (const auto* objItem = dynamic_cast<const qtBaseObjectNode*>(item))
      {
        selected.insert(objItem->object()->shared_from_this());
      }
      ++nn;
    }
  }
  if (m_p->m_arcSelectionEnabled)
  {
    for (const auto* item : m_p->m_scene->selectedItems())
    {
      if (const auto* arcItem = dynamic_cast<const qtTaskArc*>(item))
      {
        if (arcItem->adaptor())
        {
          selected.insert(arcItem->adaptor()->shared_from_this());
        }
      }
    }
  }
  didChange = smtkSelection->modifySelection(
    selected,
    m_p->m_selectionSource,
    m_p->m_selectionValue,
    smtk::view::SelectionAction::UNFILTERED_REPLACE,
    /* bitwise */ true);
  return didChange;
}

void qtDiagram::updateQtSelection()
{
  auto selection = m_p->m_managers->get<smtk::view::Selection::Ptr>();
  auto selected =
    selection->currentSelectionByValueAs<std::vector<smtk::resource::PersistentObject::Ptr>>(
      "selected", false);
  m_p->m_selectionUpdatingFromSMTK = true;
  m_p->m_scene->clearSelection();
  if (this->nodesEnabled())
  {
    for (const auto& obj : selected)
    {
      auto* node = this->findNode(obj->id());
      if (node)
      {
        node->setSelected(true);
      }
    }
  }
  // TODO: If arc selection is enabled, search for the selected arc.
  //       The problem is that some arcs are SMTK components (e.g.,
  //       task-adaptors) while some are node (e.g., graph-resource arcs).
  //       Arcs that are components conveniently have a UUID but qtBaseArc
  //       does not require a UUID, so there is no lookup.
  m_p->m_selectionUpdatingFromSMTK = false;
}

qtDiagramScene* qtDiagram::diagramScene() const
{
  return m_p->m_scene;
}

qtDiagramView* qtDiagram::diagramWidget() const
{
  return m_p ? m_p->m_view : nullptr;
}

qtDiagramLegend* qtDiagram::legend() const
{
  return m_p ? m_p->m_legend : nullptr;
}

qtBaseNode* qtDiagram::findNode(const smtk::common::UUID& uid) const
{
  auto it = m_p->m_nodeIndex.find(uid);
  if (it == m_p->m_nodeIndex.end())
  {
    return nullptr;
  }
  return it->second;
}

const std::unordered_map<
  qtBaseNode*,
  std::unordered_map<smtk::string::Token, std::unordered_set<qtBaseArc*>>>*
qtDiagram::arcsFromNode(qtBaseNode* node) const
{
  auto it = m_p->m_arcIndex.find(node);
  if (it == m_p->m_arcIndex.end())
  {
    return nullptr;
  }
  return &it->second;
}

const std::unordered_map<smtk::string::Token, std::unordered_set<qtBaseArc*>>* qtDiagram::findArcs(
  qtBaseNode* source,
  qtBaseNode* target) const
{
  const auto* originMap = this->arcsFromNode(source);
  if (!originMap)
  {
    return nullptr;
  }

  auto it = originMap->find(target);
  if (it == originMap->end())
  {
    return nullptr;
  }

  return &it->second;
}

std::unordered_set<qtBaseNode*> qtDiagram::predecessorsOf(qtBaseNode* successor) const
{
  std::unordered_set<qtBaseNode*> result;
  auto it = m_p->m_arcReverseIndex.find(successor);
  if (it == m_p->m_arcReverseIndex.end())
  {
    return result;
  }
  result.insert(it->second.begin(), it->second.end());
  return result;
}

bool qtDiagram::addNode(qtBaseNode* node, bool enforceInteractionMode)
{
  if (!node)
  {
    return false;
  }
  auto it = m_p->m_nodeIndex.find(node->nodeId());
  if (it != m_p->m_nodeIndex.end())
  {
    return false;
  }
  if (enforceInteractionMode)
  {
    node->setEnabled(m_p->m_nodesEnabled);
    if (node->flags() & QGraphicsItem::ItemIsSelectable && !m_p->m_nodeSelectionEnabled)
    {
      node->setFlags(node->flags() & ~QGraphicsItem::ItemIsSelectable);
    }
  }
  m_p->m_nodeIndex[node->nodeId()] = node;

  // If the node has a location provided by the operation, use it.
  auto cit = m_p->m_layoutMap.find(node->nodeId());
  if (cit != m_p->m_layoutMap.end())
  {
    node->setPos(cit->second[0], cit->second[1]);
  }
  return true;
}

bool qtDiagram::removeNode(qtBaseNode* node)
{
  if (!node)
  {
    return false;
  }
  auto it = m_p->m_nodeIndex.find(node->nodeId());
  if (it == m_p->m_nodeIndex.end())
  {
    return false;
  }

  auto arcsToRemove = this->arcsOfNode(node);
  for (const auto& arc : arcsToRemove)
  {
    this->removeArc(arc);
  }

  // Now remove the node from the index and scene
  m_p->m_nodeIndex.erase(it);
  this->diagramScene()->removeItem(node);
  delete node;

  return true;
}

bool qtDiagram::addArc(qtBaseArc* arc, bool enforceSelectionMode)
{
  if (!arc || !arc->predecessor() || !arc->successor())
  {
    return false;
  }
  m_p->m_arcIndex[arc->predecessor()][arc->successor()][arc->arcType()].insert(arc);
  m_p->m_arcReverseIndex[arc->successor()].insert(arc->predecessor());
  if (enforceSelectionMode)
  {
    if (m_p->m_arcSelectionEnabled)
    {
      arc->setFlags(arc->flags() | QGraphicsItem::ItemIsSelectable);
    }
  }
  return true;
}

bool qtDiagram::removeArc(qtBaseArc* arc)
{
  if (!arc)
  {
    return false;
  }
  auto pit = m_p->m_arcIndex.find(arc->predecessor());
  auto sit = m_p->m_arcReverseIndex.find(arc->successor());
  if (pit == m_p->m_arcIndex.end() || sit == m_p->m_arcReverseIndex.end())
  {
    return false;
  }
  auto pit2 = pit->second.find(arc->successor());
  if (pit2 == pit->second.end())
  {
    return false;
  }
  auto pit3 = pit2->second.find(arc->arcType());
  if (pit3 == pit2->second.end())
  {
    return false;
  }
  auto pit4 = pit3->second.find(arc);
  if (pit4 == pit3->second.end())
  {
    return false;
  }
  auto sit2 = sit->second.find(arc->predecessor());
  if (sit2 == sit->second.end())
  {
    return false;
  }
  // Erase the arc from the forward (predecessor-first) index.
  pit3->second.erase(pit4);
  if (pit3->second.empty())
  {
    pit2->second.erase(pit3);
  }
  // Now clean up if that was the only arc between predecessor and successor.
  if (pit2->second.empty())
  {
    pit->second.erase(pit2);
    sit->second.erase(sit2);
    // Now clean up if the predecessor has no more outgoing arcs.
    if (pit->second.empty())
    {
      m_p->m_arcIndex.erase(pit);
    }
  }
  // Now clean up if the successor has no more incoming arcs.
  if (sit->second.empty())
  {
    m_p->m_arcReverseIndex.erase(sit);
  }
  // Finally, now that the arc is de-indexed, delete it.
  this->diagramScene()->removeItem(arc); // FIXME: Some arcs have a null scene???
  delete arc;
  return true;
}

std::unordered_set<qtBaseArc*> qtDiagram::arcsOfNode(qtBaseNode* node)
{
  std::unordered_set<qtBaseArc*> result;
  if (!node)
  {
    return result;
  }
  // Find outgoing arcs from this node.
  auto ait = m_p->m_arcIndex.find(node);
  if (ait != m_p->m_arcIndex.end())
  {
    for (const auto& arcTypeEntry : ait->second)
    {
      for (const auto& arcSetEntry : arcTypeEntry.second)
      {
        result.insert(arcSetEntry.second.begin(), arcSetEntry.second.end());
      }
    }
  }
  // Add incoming arcs to this node.
  auto rit = m_p->m_arcReverseIndex.find(node);
  if (rit != m_p->m_arcReverseIndex.end())
  {
    for (const auto& predecessor : rit->second)
    {
      auto fit = m_p->m_arcIndex.find(predecessor);
      if (fit != m_p->m_arcIndex.end())
      {
        auto fit2 = fit->second.find(node);
        if (fit2 != fit->second.end())
        {
          for (const auto& arcTypeEntry : fit2->second)
          {
            result.insert(arcTypeEntry.second.begin(), arcTypeEntry.second.end());
          }
        }
      }
    }
  }
  return result;
}

std::shared_ptr<smtk::view::Configuration> qtDiagram::defaultConfiguration()
{
  std::shared_ptr<smtk::view::Configuration> result;
  auto jsonConfig = nlohmann::json::parse(taskPanelConfiguration())[0];
  result = jsonConfig;
  return result;
}

bool qtDiagram::configure(const nlohmann::json& data)
{
  return m_p->configure(data);
}

nlohmann::json qtDiagram::configuration() const
{
  return m_p->configuration();
}

void qtDiagram::enableNodes(bool shouldEnable)
{
  if (m_p->m_nodesEnabled == shouldEnable)
  {
    // Nothing to do.
    return;
  }

  for (const auto& entry : m_p->m_nodeIndex)
  {
    entry.second->setEnabled(shouldEnable);
  }
  m_p->m_nodesEnabled = shouldEnable;
  // TODO: Notify generators?
}

void qtDiagram::enableNodeSelection(bool shouldEnable)
{
  if (m_p->m_nodeSelectionEnabled == shouldEnable)
  {
    // Nothing to do.
    return;
  }

  if (shouldEnable)
  {
    for (const auto& entry : m_p->m_nodeIndex)
    {
      entry.second->setFlags(entry.second->flags() | QGraphicsItem::ItemIsSelectable);
    }
  }
  else
  {
    for (const auto& entry : m_p->m_nodeIndex)
    {
      entry.second->setFlags(entry.second->flags() & ~QGraphicsItem::ItemIsSelectable);
    }
  }
  m_p->m_nodeSelectionEnabled = shouldEnable;
  // TODO: Notify generators?
}

void qtDiagram::enableArcSelection(bool shouldEnable)
{
  if (m_p->m_arcSelectionEnabled == shouldEnable)
  {
    // Nothing to do.
    return;
  }

  if (shouldEnable)
  {
    // Ensure tasks cannot be selected while arcs can be.
    for (const auto& entry : m_p->m_nodeIndex)
    {
      entry.second->setFlags(entry.second->flags() & ~QGraphicsItem::ItemIsSelectable);
    }
    // Enable arc selection.
    for (const auto& predecessorEntry : m_p->m_arcIndex)
    {
      for (const auto& successorEntry : predecessorEntry.second)
      {
        for (const auto& arcs : successorEntry.second)
        {
          for (auto* arc : arcs.second)
          {
            arc->setFlags(arc->flags() | QGraphicsItem::ItemIsSelectable);
          }
        }
      }
    }
  }
  else
  {
    // Enable task selection now that arcs are no longer selectable.
    for (const auto& entry : m_p->m_nodeIndex)
    {
      entry.second->setFlags(entry.second->flags() | QGraphicsItem::ItemIsSelectable);
    }
    // Disable arc selection.
    for (const auto& predecessorEntry : m_p->m_arcIndex)
    {
      for (const auto& successorEntry : predecessorEntry.second)
      {
        for (const auto& arcs : successorEntry.second)
        {
          for (auto* arc : arcs.second)
          {
            arc->setFlags(arc->flags() & ~QGraphicsItem::ItemIsSelectable);
          }
        }
      }
    }
  }
  m_p->m_arcSelectionEnabled = shouldEnable;
  // TODO: Notify generators?
}

smtk::string::Token qtDiagram::mode() const
{
  return m_p->m_mode;
}

smtk::string::Token qtDiagram::defaultMode() const
{
  return m_p->m_defaultModeName;
}

qtDiagramViewMode* qtDiagram::modeObject() const
{
  auto it = m_p->m_modeMap.find(m_p->m_mode);
  if (it == m_p->m_modeMap.end())
  {
    return nullptr;
  }
  return it->second.get();
}

const std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramViewMode>>&
qtDiagram::modes() const
{
  return m_p->m_modeMap;
}

smtk::common::Managers::Ptr qtDiagram::managers() const
{
  return m_p->m_managers;
}

const std::unordered_map<smtk::string::Token, std::shared_ptr<qtDiagramGenerator>>&
qtDiagram::generators() const
{
  return m_p->m_generators;
}

void qtDiagram::includeInView(const QRectF& inclusion, int priority)
{
  m_p->includeInView(inclusion, priority);
}

bool qtDiagram::nodesEnabled() const
{
  return m_p->m_nodesEnabled;
}

bool qtDiagram::nodeSelectionEnabled() const
{
  return m_p->m_nodeSelectionEnabled;
}

bool qtDiagram::arcSelectionEnabled() const
{
  return m_p->m_arcSelectionEnabled;
}

QWidget* qtDiagram::sidebar() const
{
  return m_p->m_sidebarInner;
}

void qtDiagram::modeChangeRequested(QAction* modeAction)
{
  smtk::string::Token mode = (modeAction ? modeAction->objectName().toStdString() : "default");
  if (!modeAction->isChecked())
  {
    mode = "default"_token;
  }
  if (m_p->m_mode == mode)
  {
    // No change.
    return;
  }

  auto it = m_p->m_modeMap.find(m_p->m_mode);
  if (it != m_p->m_modeMap.end())
  {
    it->second->exitMode();
  }
  it = m_p->m_modeMap.find(mode);
  if (it != m_p->m_modeMap.end())
  {
    it->second->enterMode();
  }

  // Finally notify others that the mode has changed.
  m_p->m_mode = mode;
  Q_EMIT modeChanged(mode);
}

void qtDiagram::onNodeGeometryChanged()
{
  if (!m_p || !m_p->m_operationManager)
  {
    return;
  }

  // Mark project dirty (modified, in need of a save) when nodes
  // are repositioned.
  auto* taskNode = dynamic_cast<qtBaseTaskNode*>(this->sender());
  if (taskNode != nullptr)
  {
    auto rsrc = taskNode->task()->resource();
    if (rsrc && rsrc->clean())
    {
      auto marker = m_p->m_operationManager->create("smtk::operation::MarkModified");
      if (!marker)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Unable to mark \"" << rsrc->name() << "\" modified.");
        return;
      }
      marker->parameters()->associate(rsrc);
      m_p->m_operationManager->launchers()(marker);
    }
  }
}

} // namespace extension
} // namespace smtk
