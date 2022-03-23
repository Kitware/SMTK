//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkMeshInspectorView.h"
// #include "smtk/extension/paraview/operators/ui_smtkMeshInspectorView.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/widgets/pqSMTKSliceItemWidget.h"
#include "smtk/extension/qt/qtReferenceTree.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/view/Configuration.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqLinksModel.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"

#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSessionProxyManager.h"

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
#include <QVBoxLayout>

using namespace smtk::extension;

class smtkMeshInspectorView::Internals
{
public:
  Internals() = default;

  ~Internals()
  {
    this->cleanup();
    delete m_currentAtt;
  }

  void cleanup()
  {
    pqApplicationCore* paraViewApp = pqApplicationCore::instance();
    pqObjectBuilder* builder = paraViewApp->getObjectBuilder();
    if (m_crinkle)
    {
      builder->destroy(m_crinkle);
      m_crinkle = nullptr;
    }
    if (m_extract)
    {
      builder->destroy(m_extract);
      m_extract = nullptr;
    }
    // Do not delete the input data, but clear the pointer to it:
    m_geometry = nullptr;
  }

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
          (attType == "smtk::geometry::MeshInspector" ||
           attType == "smtk::geometry::ImageInspector"))
        {
          result = &child;
          break;
        }
      }
    }
    return result;
  }

  bool fetchItems(
    QPointer<pqSMTKSliceItemWidget>& sliceItem,
    QPointer<qtReferenceTree>& treeItem,
    const QList<qtItem*>& items)
  {
    int found = 0;
    for (auto* item : items)
    {
      if (auto* castToSlice = dynamic_cast<pqSMTKSliceItemWidget*>(item))
      {
        sliceItem = castToSlice;
        ++found;
      }
      else if (auto* castToTree = dynamic_cast<qtReferenceTree*>(item))
      {
        treeItem = castToTree;
        ++found;
      }
    }
    return found == 2;
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    bool haveFilters = false;
    qtAttribute* attInstance = nullptr;
    if (att && att->numberOfItems() > 0)
    {
      const smtk::view::Configuration::Component* comp =
        this->findConfigComponent(view->configuration());
      attInstance = new qtAttribute(att, *comp, pw, view);
      // attInstance->setUseSelectionManager(view->useSelectionManager());
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("meshInspectorEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
#if 1
        QPointer<pqSMTKSliceItemWidget> sliceItem;
        QPointer<qtReferenceTree> treeItem;
        if (this->fetchItems(sliceItem, treeItem, attInstance->items()))
        {
          auto assoc = att->associations();
          auto component =
            assoc->numberOfValues() > 0 ? assoc->valueAs<smtk::resource::Component>(0) : nullptr;
          auto resource = component ? component->resource() : nullptr;
          auto* behavior = pqSMTKBehavior::instance();
          m_geometry = behavior->getPVResource(resource);

          QObject::connect(
            treeItem,
            &qtReferenceTree::modified,
            sliceItem,
            &pqSMTKSliceItemWidget::sliceInputsChanged);
        }
#else
        pqApplicationCore* paraViewApp = pqApplicationCore::instance();
        pqServer* server = paraViewApp->getActiveServer();
        pqObjectBuilder* builder = paraViewApp->getObjectBuilder();
        QPointer<pqSMTKSliceItemWidget> sliceItem;
        QPointer<qtReferenceTree> treeItem;
        if (this->fetchItems(sliceItem, treeItem, attInstance->items()))
        {
          auto assoc = att->associations();
          auto component =
            assoc->numberOfValues() > 0 ? assoc->valueAs<smtk::resource::Component>(0) : nullptr;
          auto resource = component ? component->resource() : nullptr;
          auto behavior = pqSMTKBehavior::instance();
          m_geometry = behavior->getPVResource(resource);
          // m_extract = builder->createFilter("filters", "ExtractBlock", m_geometry);
          // m_crinkle = builder->createFilter("filters", "Cut", m_extract);
          m_crinkle = builder->createFilter("filters", "Cut", m_geometry);
          auto crinklePxy = m_crinkle->getProxy();
          vtkSMPropertyHelper(crinklePxy, "PreserveInputCells").Set(true);
          haveFilters = true;
          // TODO: Add button for m_crinkle "Invert" property
          // TODO: Avoid double-slice-widget (1 from item, 1 from property-panel).
          //       Idea 1: builder->createProxy() instead of builder->createFilter()?
          //       Idea 2: Custom XML without the PropertyWidgetDecorator for the implicit function?
          //       Both ideas would require a property link between item's slice and filter's slice.
          auto widgetPxy = sliceItem->propertyWidget()->widgetProxy();
          pqLinksModel* model = paraViewApp->getLinksModel();
          model->addPropertyLink("CrinklePlaneOrigin", widgetPxy, "Origin", crinklePxy, "Origin");
          model->addPropertyLink("CrinklePlaneNormal", widgetPxy, "Normal", crinklePxy, "Normal");
        }
#endif
        auto* doneButton = new QPushButton("Done");
        parentlayout->addWidget(doneButton);
        auto* opView = dynamic_cast<smtk::extension::qtOperationView*>(view);
        QObject::connect(
          doneButton,
          &QPushButton::released,
          opView,
          &smtk::extension::qtOperationView::doneEditing);
      }
    }

    if (!haveFilters)
    {
      m_geometry = nullptr;
      m_extract = nullptr;
      m_crinkle = nullptr;
    }

    return attInstance;
  }

  QPointer<qtAttribute> m_currentAtt;
  QPointer<QHBoxLayout> m_editorLayout;

  smtk::shared_ptr<smtk::operation::Operation> m_currentOp;
  QPointer<pqSMTKResource> m_geometry;
  QPointer<pqPipelineSource> m_extract;
  QPointer<pqPipelineSource> m_crinkle;
};

smtkMeshInspectorView::smtkMeshInspectorView(const smtk::view::Information& info)
  : qtOperationView(info)
{
  m_p = new Internals;
  m_p->m_currentOp = info.get<smtk::shared_ptr<smtk::operation::Operation>>();
}

smtkMeshInspectorView::~smtkMeshInspectorView()
{
  delete m_p;
}

bool smtkMeshInspectorView::displayItem(smtk::attribute::ItemPtr item) const
{
  // Here is where item visibility can be overridden for the custom view.
  return this->qtBaseAttributeView::displayItem(item);
}

qtBaseView* smtkMeshInspectorView::createViewWidget(const smtk::view::Information& info)
{
  if (qtOperationView::validateInformation(info))
  {
    auto* view = new smtkMeshInspectorView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

void smtkMeshInspectorView::attributeModified()
{
  // Always enable the apply button here.
}

void smtkMeshInspectorView::createWidget()
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
  this->Widget->setObjectName("meshInspectorView");
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  m_p->m_editorLayout = new QHBoxLayout;
  this->updateUI();

  QWidget* wtmp = new QWidget;
  layout->addWidget(wtmp);
  // Here is where we could fetch user preferences
}

void smtkMeshInspectorView::onShowCategory()
{
  this->updateUI();
}

void smtkMeshInspectorView::updateUI()
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
      if (optype == "smtk::geometry::MeshInspector" || optype == "smtk::geometry::ImageInspector")
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

void smtkMeshInspectorView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  op->operate();
}

void smtkMeshInspectorView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  // This is a method for child items to call when they are changed.
  // Normally, one would emit this->modified() and invoke
  // this->uiManager()->onViewUIModified(this, item);

  (void)valItem;
  // std::cout << "Item " << valItem->name() << " type " << valItem->type() << " changed\n";
  // this->requestOperation(m_p->m_currentOp);
}

void smtkMeshInspectorView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
