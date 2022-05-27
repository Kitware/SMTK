//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkDataSetInfoInspectorView.h"
// #include "smtk/extension/paraview/operators/ui_smtkDataSetInfoInspectorView.h"

#include "smtk/extension/qt/qtReferenceTree.h"
#include "smtk/extension/vtk/operators/DataSetInfoInspector.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/common/StringUtil.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/view/Configuration.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace smtk::extension;

class smtkDataSetInfoInspectorView::Internals
{
public:
  Internals() = default;

  ~Internals()
  {
    this->cleanup();
    delete m_currentAtt;
  }

  void cleanup() {}

  const smtk::view::Configuration::Component* findConfigComponent(
    const smtk::view::ConfigurationPtr& config)
  {
    static thread_local smtk::view::Configuration::Component dummy;
    const smtk::view::Configuration::Component* result = &dummy;
    const auto& details = config->details();
    int attributeTypesIndex = -1;
    if ((attributeTypesIndex = details.findChild("AttributeTypes")) >= 0)
    {
      const auto& attributeTypes = details.child(attributeTypesIndex);
      for (const auto& child : attributeTypes.children())
      {
        std::string attType;
        if (
          child.name() == "Att" && child.attribute("Type", attType) &&
          (attType == "smtk::geometry::DataSetInfoInspector" ||
           attType == "smtk::geometry::ImageInspector"))
        {
          result = &child;
          break;
        }
      }
    }
    return result;
  }

  bool fetchItems(QPointer<qtReferenceTree>& treeItem, const QList<qtItem*>& items)
  {
    int found = 0;
    for (auto* item : items)
    {
      if (auto* castToTree = dynamic_cast<qtReferenceTree*>(item))
      {
        treeItem = castToTree;
        ++found;
      }
    }
    return found == 1;
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    qtAttribute* attInstance = nullptr;
    if (att && att->numberOfItems() > 0)
    {
      const smtk::view::Configuration::Component* comp =
        this->findConfigComponent(view->configuration());
      attInstance = new qtAttribute(att, *comp, pw, view);
      // attInstance->setUseSelectionManager(view->useSelectionManager());
      if (attInstance && attInstance->widget())
      {
        // Without any additional info lets use a basic layout with associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("dataSetInfoInspectorEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
        QPointer<qtReferenceTree> treeItem;
        if (this->fetchItems(treeItem, attInstance->items()))
        {
          auto assoc = att->associations();
          auto component =
            assoc->numberOfValues() > 0 ? assoc->valueAs<smtk::resource::Component>(0) : nullptr;
          auto resource = component ? component->resource() : nullptr;

          // Monitor the associations and invoke inputsChanged() when modified.
          // This will cause the operation to run and results to be updated.
          QObject::connect(
            treeItem,
            &qtReferenceTree::modified,
            dynamic_cast<smtkDataSetInfoInspectorView*>(view),
            &smtkDataSetInfoInspectorView::inputsChanged);
        }
      }
    }

    return attInstance;
  }

  QPointer<qtAttribute> m_currentAtt;
  QPointer<QHBoxLayout> m_editorLayout;
  QPointer<QTableWidget> m_summaryTable;

  smtk::shared_ptr<smtk::operation::Operation> m_currentOp;
  smtk::operation::Observers::Key m_opObserver;
};

smtkDataSetInfoInspectorView::smtkDataSetInfoInspectorView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  m_p = new Internals;
  m_p->m_currentOp = info.get<smtk::shared_ptr<smtk::operation::Operation>>();
  // If we have an operation and it's configured properly, run it now:
  if (m_p->m_currentOp && m_p->m_currentOp->parameters()->associations()->numberOfValues() > 0)
  {
    this->requestOperation(m_p->m_currentOp);
  }
}

smtkDataSetInfoInspectorView::~smtkDataSetInfoInspectorView()
{
  delete m_p;
}

bool smtkDataSetInfoInspectorView::displayItem(smtk::attribute::ItemPtr item) const
{
  // Here is where item visibility can be overridden for the custom view.
  return this->qtBaseAttributeView::displayItem(item);
}

qtBaseView* smtkDataSetInfoInspectorView::createViewWidget(const smtk::view::Information& info)
{
  if (qtBaseAttributeView::validateInformation(info))
  {
    auto* view = new smtkDataSetInfoInspectorView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

void smtkDataSetInfoInspectorView::inputsChanged()
{
  this->requestOperation(m_p->m_currentOp);
  // Always enable the apply button here.
}

void smtkDataSetInfoInspectorView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  // Delete any pre-existing widget
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  // Create new pipeline filters

  // I. Create the ParaView widget and a proxy for its representation.

  // Create a new frame and lay it out
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName("dataSetInfoInspectorView");
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  m_p->m_editorLayout = new QHBoxLayout;
  m_p->m_summaryTable = new QTableWidget;
  m_p->m_summaryTable->setObjectName("DataSetInfo");
  m_p->m_summaryTable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  this->updateUI();

  QWidget* wtmp = new QWidget;
  layout->addWidget(wtmp);
  layout->addWidget(m_p->m_summaryTable);
  // Here is where we could fetch user preferences
}

void smtkDataSetInfoInspectorView::onShowCategory()
{
  this->updateUI();
}

void smtkDataSetInfoInspectorView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view || !this->Widget)
  {
    return;
  }

  // Fetch configuration information and apply it.
  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
  {
    return;
  }
  smtk::view::Configuration::Component& comp = view->details().child(i);
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
  {
    smtk::view::Configuration::Component& attComp = comp.child(ci);
    if (attComp.name() != "Att")
    {
      continue;
    }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
    {
      if (
        optype == "smtk::geometry::DataSetInfoInspector" ||
        optype == "smtk::geometry::ImageInspector")
      {
        defName = optype;
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  // expecting only 1 instance of the op?
  // smtk::attribute::AttributePtr att = this->operation()->parameters();
  auto op = m_viewInfo.get<smtk::operation::OperationPtr>();
  auto att = op ? op->parameters() : nullptr;
  m_p->m_currentAtt = m_p->createAttUI(att, this->Widget, this);
}

void smtkDataSetInfoInspectorView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  auto opManager = op->manager();
  if (!m_p->m_opObserver.assigned() && opManager)
  {
    smtk::operation::Observer observer = [this](
                                           const smtk::operation::Operation& op,
                                           smtk::operation::EventType event,
                                           smtk::operation::Operation::Result result) {
      if (
        event == smtk::operation::EventType::DID_OPERATE &&
        dynamic_cast<const smtk::geometry::DataSetInfoInspector*>(&op))
      {
        this->updateInfoTable(result);
      }
      return 0; // 0 = do not cancel operation
    };
    m_p->m_opObserver = opManager->observers().insert(
      observer,
      std::numeric_limits<smtk::operation::Observers::Priority>::lowest(),
      /* initialize */ false,
      /* description */ "Update info inspector");
  }
  if (auto operationManager = op->manager())
  {
    operationManager->launchers()(op);
    // The result will be made available to our operation observer
  }
  else
  {
    auto result = op->operate();
    // Urk, we want to run asynchronously
    if (!m_p->m_opObserver.assigned())
    {
      this->updateInfoTable(result);
    }
  }
}

void nonEmptyCounters(
  const std::shared_ptr<smtk::attribute::GroupItem>& datasets,
  std::vector<std::size_t>& compIndices,
  std::vector<std::size_t>& itemIndices,
  QStringList& infoNames)
{
  struct Comparator
  {
    bool operator()(const std::string& aa, const std::string& bb) const
    {
      return smtk::common::StringUtil::mixedAlphanumericComparator(aa, bb);
    }
  };

  bool haveBounds = false;
  std::set<std::size_t> nonZeroCounts;
  std::map<std::string, std::size_t, Comparator> sortedComponents;
  for (std::size_t ii = 0; ii < datasets->numberOfGroups(); ++ii)
  {
    auto name = datasets->findAs<smtk::attribute::ComponentItem>(ii, "component")->value()->name();
    sortedComponents[name] = ii;
    if (!haveBounds)
    {
      auto boundsItem = datasets->findAs<smtk::attribute::DoubleItem>(ii, "bounds");
      haveBounds = boundsItem->isSet();
    }
    for (std::size_t jj = 0; jj < datasets->numberOfItemsPerGroup(); ++jj)
    {
      if (auto item = std::dynamic_pointer_cast<smtk::attribute::IntItem>(datasets->item(ii, jj)))
      {
        if (item->isSet() && item->value() > 0)
        {
          nonZeroCounts.insert(jj);
        }
      }
    }
  }
  compIndices.clear();
  itemIndices.clear();
  for (const auto& entry : sortedComponents)
  {
    compIndices.push_back(entry.second);
  }
  if (haveBounds)
  {
    infoNames.push_back("Δx");
    infoNames.push_back("Δy");
    infoNames.push_back("Δz");
  }
  for (const auto& index : nonZeroCounts)
  {
#if 0
    std::cout
      << std::dynamic_pointer_cast<smtk::attribute::IntItem>(datasets->item(0, index))->label() << " "
      << std::dynamic_pointer_cast<smtk::attribute::IntItem>(datasets->item(0, index))->value() << " "
      << index << "\n";
#endif
    itemIndices.push_back(index);
    infoNames.push_back(QString::fromStdString(datasets->item(0, index)->label()));
  }
}

void smtkDataSetInfoInspectorView::updateInfoTable(const smtk::attribute::AttributePtr& result)
{
  if (!result)
  {
    return;
  }
  auto datasets = result->findGroup("information");
  if (!datasets)
  {
    return;
  }
  // Across all groups, find the set of non-empty counters
  QStringList infoNames;
  std::vector<std::size_t> itemIndices; // non-empty counter indices
  std::vector<std::size_t> compIndices; // group indices sorted by component name
  nonEmptyCounters(datasets, compIndices, itemIndices, infoNames);
  m_p->m_summaryTable->clear();
  m_p->m_summaryTable->setColumnCount(static_cast<int>(compIndices.size()));
  m_p->m_summaryTable->setRowCount(static_cast<int>(infoNames.size()));
  QStringList componentNames;
  int column = 0;
  for (const auto& compIndex : compIndices)
  {
    componentNames.push_back(QString::fromStdString(
      datasets->findAs<smtk::attribute::ComponentItem>(compIndex, "component")->value()->name()));
    auto boundsItem = datasets->findAs<smtk::attribute::DoubleItem>(compIndex, "bounds");
    int row = 0;
    if (boundsItem && boundsItem->isSet())
    {
      auto* itemWidget =
        new QTableWidgetItem(tr("%1 (%2 → %3)")
                               .arg(
                                 QString::number(boundsItem->value(1) - boundsItem->value(0)),
                                 QString::number(boundsItem->value(0)),
                                 QString::number(boundsItem->value(1))));
      itemWidget->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      m_p->m_summaryTable->setItem(row, column, itemWidget);
      ++row;

      itemWidget =
        new QTableWidgetItem(tr("%1 (%2 → %3)")
                               .arg(
                                 QString::number(boundsItem->value(3) - boundsItem->value(2)),
                                 QString::number(boundsItem->value(2)),
                                 QString::number(boundsItem->value(3))));
      itemWidget->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      m_p->m_summaryTable->setItem(row, column, itemWidget);
      ++row;

      itemWidget =
        new QTableWidgetItem(tr("%1 (%2 → %3)")
                               .arg(
                                 QString::number(boundsItem->value(5) - boundsItem->value(4)),
                                 QString::number(boundsItem->value(4)),
                                 QString::number(boundsItem->value(5))));
      itemWidget->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      m_p->m_summaryTable->setItem(row, column, itemWidget);
      ++row;
    }
    else if (infoNames[0] == "Δx")
    {
      // This component doesn't have bounds, but some other one does.
      row += 3;
    }
    for (const auto& itemIndex : itemIndices)
    {
      auto countItem =
        std::dynamic_pointer_cast<smtk::attribute::IntItem>(datasets->item(compIndex, itemIndex));
      if (countItem->isSet())
      {
        auto* itemWidget = new QTableWidgetItem(tr("%1").arg(countItem->value()));
        // Allow copying but not editing of table entries:
        itemWidget->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_p->m_summaryTable->setItem(row, column, itemWidget);
      }
      ++row;
    }
    ++column;
  }
  m_p->m_summaryTable->setHorizontalHeaderLabels(componentNames);
  m_p->m_summaryTable->setVerticalHeaderLabels(infoNames);
  m_p->m_summaryTable->resizeColumnsToContents();
  m_p->m_summaryTable->setShowGrid(false);
}
