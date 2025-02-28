//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkCoordinateTransformView.h"
#include "smtk/operation/operators/CoordinateTransform.h"

#include "smtk/extension/paraview/operators/ui_smtkCoordinateTransformView.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/common/Managers.h"
#include "smtk/common/StringUtil.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeItemInfo.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Properties.h"
#include "smtk/resource/properties/CoordinateFrame.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"
#include "smtk/view/SelectionObserver.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPresetDialog.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"
#include "smtk/extension/paraview/widgets/pqSMTKCoordinateFrameItemWidget.h"
#include "smtk/extension/qt/qtReferenceItem.h"
#include "vtkBoundingBox.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMParaViewPipelineController.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
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
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace
{
void pqAdjustBounds(vtkBoundingBox& bbox, double scaleFactor)
{
  double max_length = bbox.GetMaxLength();
  max_length = max_length > 0 ? max_length * 0.05 : 1;
  double min_point[3], max_point[3];
  bbox.GetMinPoint(min_point[0], min_point[1], min_point[2]);
  bbox.GetMaxPoint(max_point[0], max_point[1], max_point[2]);
  for (int cc = 0; cc < 3; cc++)
  {
    if (bbox.GetLength(cc) == 0)
    {
      min_point[cc] -= max_length;
      max_point[cc] += max_length;
    }

    double mid = (min_point[cc] + max_point[cc]) / 2.0;
    min_point[cc] = mid + scaleFactor * (min_point[cc] - mid);
    max_point[cc] = mid + scaleFactor * (max_point[cc] - mid);
  }
  bbox.SetMinPoint(min_point);
  bbox.SetMaxPoint(max_point);
}
} // namespace

// On Windows MSVC 2015+, something is included that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

using namespace smtk::extension;
using smtk::attribute::DoubleItem;
using smtk::attribute::ReferenceItem;
using smtk::resource::properties::CoordinateFrame;

namespace
{
// This is a custom item used to display landmark coordinate-frames that have been
// created via the "freeform properties" operation. Users can apply these landmarks
// as the source or destination location+orientation of the transform.
class FrameTreeItem : public QTreeWidgetItem
{
public:
  FrameTreeItem(QTreeWidget* parent, smtk::resource::Resource* rsrc);
  FrameTreeItem(
    FrameTreeItem* parentItem,
    smtk::resource::Resource* rsrc,
    smtk::resource::PersistentObject* obj,
    const std::string& pname);

  FrameTreeItem(const FrameTreeItem&) = default;
  FrameTreeItem& operator=(const FrameTreeItem&) = default;

  smtk::resource::Resource* resource() const { return m_resource; }
  smtk::resource::PersistentObject* object() const { return m_object; }
  std::string propertyName() const { return m_propertyName; }

  bool operator<(const FrameTreeItem* other) const
  {
    if (!other)
    {
      return false;
    }
    if (smtk::common::StringUtil::mixedAlphanumericComparator(
          m_resource->name(), other->resource()->name()))
    {
      return true;
    }
    else if (other->resource()->name() == m_resource->name())
    {
      if (!m_object && other->object())
      {
        return true;
      }
      else if (!other->object() && m_object)
      {
        return false;
      }
      else if (other->object() == m_object)
      {
        return smtk::common::StringUtil::mixedAlphanumericComparator(
          m_propertyName, other->propertyName());
      }
      return smtk::common::StringUtil::mixedAlphanumericComparator(
        m_object->name(), other->object()->name());
    }
    return false;
  }

protected:
  smtk::resource::Resource* m_resource;
  smtk::resource::PersistentObject* m_object;
  std::string m_propertyName;
};

FrameTreeItem::FrameTreeItem(QTreeWidget* parent, smtk::resource::Resource* rsrc)
  : QTreeWidgetItem(parent, QTreeWidgetItem::UserType + 37)
  , m_resource(rsrc)
  , m_object(nullptr)
{
  this->setFlags(Qt::ItemIsEnabled);
  this->setText(0, QString::fromStdString(m_resource->name()));
  this->setExpanded(true);
}

FrameTreeItem::FrameTreeItem(
  FrameTreeItem* parentItem,
  smtk::resource::Resource* rsrc,
  smtk::resource::PersistentObject* obj,
  const std::string& pname)
  : QTreeWidgetItem(parentItem, QTreeWidgetItem::UserType + 37)
  , m_resource(rsrc)
  , m_object(obj)
  , m_propertyName(pname)
{
  if (!m_propertyName.empty())
  {
    this->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    this->setText(0, QString::fromStdString(m_propertyName));
  }
  else
  {
    this->setFlags(Qt::ItemIsEnabled);
    this->setText(0, QString::fromStdString(m_object ? m_object->name() : m_resource->name()));
    this->setExpanded(true);
  }
}

// Copy a std::array<double, Size> into an smtk::attribute::DoubleItem.
// This function returns true only when the DoubleItem was modified.
template<std::size_t Size>
bool copyVector(const std::array<double, Size>& data, smtk::attribute::DoubleItem::Ptr item)
{
  bool didChange = false;
  if (!item->setNumberOfValues(Size))
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot resize item to match.");
    return didChange;
  }
  for (std::size_t ii = 0; ii < Size; ++ii)
  {
    didChange |= (item->value(ii) != data[ii]);
    item->setValue(ii, data[ii]);
  }
  return didChange;
}

// Copy a coordinate-frame parent-UUID into a reference item.
// This requires a resource manager to convert the UUID into
// a persistent-object pointer.
bool copyParent(
  const smtk::common::UUID& parentId,
  const smtk::attribute::ReferenceItem::Ptr& parentItem,
  const smtk::resource::Resource::Ptr& resource,
  const smtk::resource::Manager::Ptr& resourceManager)
{
  bool didChange = false;
  if (parentId.isNull())
  {
    didChange |= (parentItem->isSet() && !parentItem->value());
    parentItem->unset();
    return didChange;
  }
  // Lookup could be expensive... check that the existing parent does
  // not have the same ID first.
  if (parentItem->isSet() && parentItem->value() && parentItem->value()->id() == parentId)
  {
    return didChange;
  }

  // Check first in the resource we are given for the parentId:
  smtk::resource::PersistentObject::Ptr parent = resource ? resource->find(parentId) : nullptr;
  if (!parent)
  {
    // Now check every resource in the resource manager:
    parent = resourceManager->search(parentId);
  }
  if (!parent)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find object for parent ID " << parentId << " in any managed resource.");
    return didChange;
  }
  didChange = true; // We already checked above that the values did not match.
  parentItem->setValue(parent);
  return didChange;
}

} // namespace

// This internal class holds the Qt UI components (from smtkCoordinateTransformView.ui)
// as well as additional state used to render coordinate frames in the 3-D view.
class smtkCoordinateTransformView::Internals : public Ui::smtkCoordinateTransformView
{
public:
  Internals(::smtkCoordinateTransformView* opView, const smtk::view::Information& info)
    : m_currentOp(std::dynamic_pointer_cast<smtk::operation::CoordinateTransform>(
        info.get<smtk::operation::Operation::Ptr>()))
    // , m_previewProxy(nullptr)
    , m_opView(opView)
  {
    const auto* viewConfig = info.configuration();
    if (viewConfig)
    {
      const auto& viewComp = viewConfig->details();
      int attributeTypesIdx = viewComp.findChild("AttributeTypes");
      if (attributeTypesIdx >= 0)
      {
        const auto& attributeViewTypes = viewComp.child(attributeTypesIdx);
        for (const auto& viewTypeEntry : attributeViewTypes.children())
        {
          std::string attName;
          if (
            viewTypeEntry.name() != "Att" || !viewTypeEntry.attribute("Name", attName) ||
            attName != "coordinate transform")
          {
            continue;
          }
          int childIdx = viewTypeEntry.findChild("ItemViews");
          if (childIdx >= 0)
          {
            const auto& iviews = viewTypeEntry.child(childIdx);
            qtAttributeItemInfo::buildFromComponent(iviews, m_opView, m_itemViewMap);
          }
        }
      }
    }

    if (m_currentOp)
    {
      auto coordinateFrameGroup = std::dynamic_pointer_cast<smtk::attribute::GroupItem>(
        m_currentOp->parameters()->find("from", smtk::attribute::SearchStyle::IMMEDIATE));
      m_widgetCoordinateFrameGroupItem = coordinateFrameGroup;
      m_coordinateSystemParent = m_widgetCoordinateFrameGroupItem
        ? m_widgetCoordinateFrameGroupItem->findAs<ReferenceItem>("Parent")
        : nullptr;
    }
    else
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create a transform operation.");
    }
    auto* app = pqApplicationCore::instance();
    auto* builder = app->getObjectBuilder();
    m_frameProxy = builder->createProxy(
      "representations", "CoordinateFrameWidgetRepresentation", app->getActiveServer(), "");
    /*
    m_previewProxy = builder->createProxy(
      "representations", "CoordinateTransformPreviewWidgetRepresentation", app->getActiveServer(), "");
      */
    vtkNew<vtkSMParaViewPipelineController> controller;
    controller->InitializeProxy(m_frameProxy);
    // controller->InitializeProxy(m_previewProxy);
    auto* view = pqActiveObjects::instance().activeView();
    if (view)
    {
      vtkSMPropertyHelper(view->getProxy(), "HiddenRepresentations").Add(m_frameProxy);
      // vtkSMPropertyHelper(view->getProxy(), "HiddenRepresentations").Add(m_previewProxy);
      view->getProxy()->UpdateVTKObjects();
      view->render();
    }

    vtkBoundingBox bbox(0, 1, 0, 1, 0, 1);
    vtkSMNewWidgetRepresentationProxy* wdgProxy =
      vtkSMNewWidgetRepresentationProxy::SafeDownCast(m_frameProxy);
    double scaleFactor = 1.05; // vtkSMPropertyHelper(wdgProxy, "PlaceFactor").GetAsDouble();
    pqAdjustBounds(bbox, scaleFactor);
    double bds[6];
    bbox.GetBounds(bds);
    vtkSMPropertyHelper(wdgProxy, "WidgetBounds").Set(bds, 6);
    vtkSMPropertyHelper(wdgProxy, "Enabled").Set(true);
    // wdgProxy->UpdateVTKObjects();
  }

  ~Internals()
  {
    auto* view = pqActiveObjects::instance().activeView();
    if (view)
    {
      vtkSMPropertyHelper(view->getProxy(), "HiddenRepresentations").Remove(m_frameProxy);
      view->getProxy()->UpdateVTKObjects();
      view->render();
    }

    vtkSMSessionProxyManager* pxm = m_frameProxy->GetSessionProxyManager();
    pxm->UnRegisterProxy(m_frameProxy);
  }

  // Add actions from the Qt UI file to various widgets
  void installActions()
  {
    // m_utilityMenuButton->addAction(m_actionLivePreview);
    m_utilityMenuButton->addAction(m_actionUseLandmarkAsFrom);
    m_utilityMenuButton->addAction(m_actionUseLandmarkAsTo);
    m_utilityMenuButton->addAction(m_actionRemoveTransform);
    m_frameTree->addAction(m_actionUseLandmarkAsFrom);
    m_frameTree->addAction(m_actionUseLandmarkAsTo);

    QObject::connect(m_actionLivePreview, &QAction::toggled, [this](bool showPreview) {
      m_opView->togglePreview(showPreview);
    });

    QObject::connect(
      m_actionUseLandmarkAsFrom,
      &QAction::triggered,
      m_opView,
      &::smtkCoordinateTransformView::useLandmarkAsFrom);
    QObject::connect(
      m_actionUseLandmarkAsTo,
      &QAction::triggered,
      m_opView,
      &::smtkCoordinateTransformView::useLandmarkAsTo);

    QObject::connect(
      m_actionRemoveTransform,
      &QAction::triggered,
      m_opView,
      &::smtkCoordinateTransformView::removeTransform);

    // Not exactly an action... but run the operation when asked:
    QObject::connect(
      m_applyButton,
      &QPushButton::clicked,
      m_opView,
      &::smtkCoordinateTransformView::applyTransform);
  }

  void installObserver()
  {
    auto opMgr = m_opView->uiManager()->operationManager();
    if (!opMgr)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "No operation manager!");
      return;
    }
    m_operationObserver = opMgr->observers().insert(
      [this](
        const smtk::operation::Operation& op,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result result) {
        (void)op;
        if (event == smtk::operation::EventType::DID_OPERATE)
        {
          auto status =
            static_cast<smtk::operation::Operation::Outcome>(result->findInt("outcome")->value());
          if (status == smtk::operation::Operation::Outcome::SUCCEEDED)
          {
            this->resetFrameTree();
          }
          if (&op == m_currentOp.get())
          {
            // If we just added a transform, leave the apply button disabled
            // but enable the remove-transform action. If we just removed a
            // transform, do the reverse:
            bool opAddedTransform = !op.parameters()->findVoid("remove")->isEnabled();
            m_applyButton->setEnabled(!opAddedTransform);
            m_actionRemoveTransform->setEnabled(opAddedTransform);
          }
        }
        return 0;
      },
      /* priority */ 0,
      /* initialize */ false,
      "Coordinate-transform view observer.");
    this->resetFrameTree();
  }

  void constructItemViews()
  {
    auto it = m_itemViewMap.find("source");
    qtAttributeItemInfo info;
    if (it != m_itemViewMap.end())
    {
      info = it->second;
      info.setItem(m_currentOp->parameters()->associations());
    }
    else
    {
      info = qtAttributeItemInfo(
        m_currentOp->parameters()->associations(),
        smtk::view::Configuration::Component("source"),
        m_referenceItemContainer,
        m_opView);
    }
    m_assocItemLayout = new QVBoxLayout(m_referenceItemContainer);
    m_assocItemWidget = m_opView->uiManager()->createItem(info);
    m_assocItemLayout->addWidget(m_assocItemWidget->widget());
    QObject::connect(
      m_assocItemWidget,
      &qtItem::modified,
      m_opView,
      &::smtkCoordinateTransformView::associationChanged);

    it = m_itemViewMap.find("from");
    if (it != m_itemViewMap.end())
    {
      info = it->second;
      info.setItem(m_currentOp->parameters()->findGroup("from"));
    }
    else
    {
      info = qtAttributeItemInfo(
        m_currentOp->parameters()->findGroup("from"),
        smtk::view::Configuration::Component("from"),
        m_fromFrame,
        m_opView);
    }
    m_fromItemLayout = new QVBoxLayout(m_fromFrame);
    m_fromItemWidget = m_opView->uiManager()->createItem(info);
    if (!m_fromItemWidget)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create \"from\" widget.");
      return;
    }
    m_fromItemLayout->addWidget(m_fromItemWidget->widget());
    QObject::connect(
      m_fromItemWidget,
      &qtItem::modified,
      m_opView,
      &::smtkCoordinateTransformView::userEditedFromFrame);

    it = m_itemViewMap.find("to");
    if (it != m_itemViewMap.end())
    {
      info = it->second;
      info.setItem(m_currentOp->parameters()->findGroup("to"));
    }
    else
    {
      info = qtAttributeItemInfo(
        m_currentOp->parameters()->findGroup("to"),
        smtk::view::Configuration::Component("to"),
        m_toFrame,
        m_opView);
    }
    m_toItemLayout = new QVBoxLayout(m_toFrame);
    m_toItemWidget = m_opView->uiManager()->createItem(info);
    if (!m_toItemWidget)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create \"to\" widget.");
      return;
    }
    m_toItemLayout->addWidget(m_toItemWidget->widget());
    QObject::connect(
      m_toItemWidget,
      &qtItem::modified,
      m_opView,
      &::smtkCoordinateTransformView::userEditedToFrame);
  }

#if 0
  void selectionModified(const std::string& /*source*/,
    const std::shared_ptr<smtk::view::Selection>& selection)
  {
    m_objects.clear();
    selection->currentSelectionByValue(m_objects, "selected", false);
    // TODO: Update preview pipeline and/or turn off preview.
  }
#endif

  void resetFrameTree()
  {
    auto rsrcMgr = m_opView->uiManager()->resourceManager();
    m_frameTree->clear();
    if (rsrcMgr)
    {
      // For each resource, obtain the map of coordinate-frame properties:
      //   For each unique coordinate-frame property-name, visit all objects which use it:
      //     Add an entry to m_frameTree for the (resource, object, name) tuple and its parents.
      rsrcMgr->visit([this](const smtk::resource::Resource& constRsrc) {
        auto* rsrc = const_cast<smtk::resource::Resource*>(&constRsrc);
        auto& pd =
          rsrc->properties()
            .data()
            .get<
              smtk::resource::Properties::Indexed<smtk::resource::properties::CoordinateFrame>>();
        for (const auto& aa : pd.data())
        {
          if (aa.first == "transform" || aa.first == "smtk.geometry.transform")
          {
            continue;
          }
          for (const auto& bb : aa.second)
          {
            auto* comp = rsrc->component(bb.first);
            this->addTreeEntry(rsrc, comp, aa.first);
          }
        }
        return smtk::common::Processing::CONTINUE;
      });
    }
  }

  FrameTreeItem* findOrCreateFrameParentItem(
    smtk::resource::Resource* rsrc,
    smtk::resource::Component* comp)
  {
    // TODO: This can be accelerated by an index and should
    //       be when there are many landmarks. Currently it is
    //       1 or 2 linear searches.
    int numResources = m_frameTree->topLevelItemCount();
    FrameTreeItem* rsrcItem = nullptr;
    FrameTreeItem* objItem = nullptr;
    for (int ii = 0; ii < numResources; ++ii)
    {
      rsrcItem = dynamic_cast<FrameTreeItem*>(m_frameTree->topLevelItem(ii));
      if (rsrcItem && rsrcItem->resource() == rsrc)
      {
        if (!comp)
        {
          break;
        }
        int numObjects = rsrcItem->childCount();
        for (int jj = 0; jj < numObjects; ++jj)
        {
          objItem = dynamic_cast<FrameTreeItem*>(rsrcItem->child(jj));
          if (objItem && objItem->object() == comp)
          {
            return objItem;
          }
        }
      }
    }
    // We need to create a resource and/or object item.
    if (!rsrcItem)
    {
      rsrcItem = new FrameTreeItem(m_frameTree, rsrc);
      if (!comp)
      {
        return rsrcItem;
      }
    }
    objItem = new FrameTreeItem(rsrcItem, rsrc, comp, std::string());
    return objItem;
  }

  void addTreeEntry(
    smtk::resource::Resource* rsrc,
    smtk::resource::Component* comp,
    const std::string& name)
  {
    if (auto* parentItem = this->findOrCreateFrameParentItem(rsrc, comp))
    {
      auto* frameItem = new FrameTreeItem(parentItem, rsrc, comp, name);
      (void)frameItem;
    }
  }

  void copyFrameToViewProxy(const smtk::resource::properties::CoordinateFrame& frame)
  {
    // TODO: If parent is set, concatenate its transform to \a frame.
    vtkSMPropertyHelper(m_frameProxy, "Origin").Set(frame.origin.data(), 3);
    vtkSMPropertyHelper(m_frameProxy, "XAxis").Set(frame.xAxis.data(), 3);
    vtkSMPropertyHelper(m_frameProxy, "YAxis").Set(frame.yAxis.data(), 3);
    vtkSMPropertyHelper(m_frameProxy, "ZAxis").Set(frame.zAxis.data(), 3);
    m_frameProxy->UpdateVTKObjects();
    pqActiveObjects::instance().activeView()->render();
  }

  void copyFrameToDestination(
    const smtk::resource::properties::CoordinateFrame& frame,
    const smtk::attribute::GroupItem::Ptr& destination,
    qtItem* itemWidget)
  {
    smtk::resource::Resource::Ptr resource;
    // Grab the first resource from the associated objects that we find:
    for (const auto& obj : m_objects)
    {
      if (!!(resource = std::dynamic_pointer_cast<smtk::resource::Resource>(obj)))
      {
        break;
      }
      else
      {
        auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
        if (comp && !!(resource = comp->resource()))
        {
          break;
        }
      }
    }
    auto resourceManager = m_opView->uiManager()->resourceManager();
    bool didChange = false;
    didChange |= copyVector(frame.origin, destination->findAs<DoubleItem>(0, "origin"));
    didChange |= copyVector(frame.xAxis, destination->findAs<DoubleItem>(0, "x axis"));
    didChange |= copyVector(frame.yAxis, destination->findAs<DoubleItem>(0, "y axis"));
    didChange |= copyVector(frame.zAxis, destination->findAs<DoubleItem>(0, "z axis"));
    // We need a resource to find a component pointer from the frame.parent UUID.
    didChange |= copyParent(
      frame.parent, destination->findAs<ReferenceItem>(0, "parent"), resource, resourceManager);
    if (didChange)
    {
      auto* frameItemWidget = dynamic_cast<pqSMTKCoordinateFrameItemWidget*>(itemWidget);
      frameItemWidget->updateWidgetFromItem();
    }
    // Now, since we don't want to confuse the user with 2 coordinate frames on
    // top of one another, unselect all items in m_frameTree, which makes the
    // frame disappear.
    m_frameTree->setCurrentItem(nullptr);
  }

  void copyLandmarkProvenance(
    const std::string& fromOrTo,
    smtk::resource::PersistentObject* object,
    const std::string& name)
  {
    auto landmarkGroup = m_currentOp->parameters()->itemAtPathAs<smtk::attribute::GroupItem>(
      "/" + fromOrTo + "/0/landmark");
    auto landmarkObject = landmarkGroup->findAs<smtk::attribute::ReferenceItem>("object");
    auto landmarkName = landmarkGroup->findAs<smtk::attribute::StringItem>("property name");
    landmarkGroup->setIsEnabled(true);
    landmarkObject->setNumberOfValues(1);
    landmarkObject->setValue(object->shared_from_this());
    landmarkName->setValue(name);
  }

  void disableLandmarkProvenance(const std::string& fromOrTo)
  {
    auto landmarkGroup = m_currentOp->parameters()->itemAtPathAs<smtk::attribute::GroupItem>(
      "/" + fromOrTo + "/0/landmark");
    auto landmarkObject = landmarkGroup->findAs<smtk::attribute::ReferenceItem>("object");
    auto landmarkName = landmarkGroup->findAs<smtk::attribute::StringItem>("property name");
    landmarkObject->unset();
    landmarkName->unset();
    landmarkGroup->setIsEnabled(false);
  }

  void checkLandmarkProvenance(const std::string& fromOrTo, const std::string& note)
  {
    auto landmarkGroup = m_currentOp->parameters()->itemAtPathAs<smtk::attribute::GroupItem>(
      "/" + fromOrTo + "/0/landmark");
    auto landmarkObject = landmarkGroup->findAs<smtk::attribute::ReferenceItem>("object");
    auto landmarkName = landmarkGroup->findAs<smtk::attribute::StringItem>("property name");
    if (!landmarkGroup->isEnabled() && !note.empty())
    {
      // A modified landmark is being used for the frame; if the user has typed a note,
      // then save it and enable the provenance, but with no object referenced.
      landmarkGroup->setIsEnabled(true);
      landmarkObject->unset();
      landmarkObject->setNumberOfValues(0);
      landmarkName->setValue(note);
    }
  }

  bool frameFromTree(FrameTreeItem* item, smtk::resource::properties::CoordinateFrame& frame)
  {
    if (!item || item->propertyName().empty())
    {
      return false;
    }
    auto* rsrc = dynamic_cast<smtk::resource::Resource*>(item->object());
    auto* comp = dynamic_cast<smtk::resource::Component*>(item->object());
    if (rsrc)
    {
      frame =
        rsrc->properties().at<smtk::resource::properties::CoordinateFrame>(item->propertyName());
      return true;
    }
    else if (comp)
    {
      frame =
        comp->properties().at<smtk::resource::properties::CoordinateFrame>(item->propertyName());
      return true;
    }
    return false;
  }

  bool m_setUp{ false };
  QPointer<QHBoxLayout> m_editorLayout;
  std::shared_ptr<smtk::operation::CoordinateTransform> m_currentOp;
  smtk::view::SelectionObservers::Key m_selectionObserver;
  std::set<std::shared_ptr<smtk::resource::PersistentObject>> m_objects;
  std::shared_ptr<pqSMTKCoordinateFrameItemWidget::qtItem> m_frameWidget;
  std::shared_ptr<smtk::extension::qtReferenceItem> m_coordinateSystemParentWidget;
  smtk::attribute::GroupItemPtr m_widgetCoordinateFrameGroupItem = nullptr;
  smtk::attribute::ReferenceItemPtr m_coordinateSystemParent = nullptr;
  vtkSMProxy* m_frameProxy{ nullptr }; // The proxy representing landmark coordinate frames.
  // vtkSMProxy* m_previewProxy; // The proxy representing preview geometry
  std::map<std::string, qtAttributeItemInfo> m_itemViewMap;
  ::smtkCoordinateTransformView* m_opView;
  QPointer<QVBoxLayout> m_assocItemLayout;
  QPointer<qtItem> m_assocItemWidget;
  QPointer<QVBoxLayout> m_fromItemLayout;
  QPointer<qtItem> m_fromItemWidget;
  bool m_fromUsingLandmark{ false };
  QPointer<QVBoxLayout> m_toItemLayout;
  QPointer<qtItem> m_toItemWidget;
  bool m_toUsingLandmark{ false };
  smtk::operation::Observers::Key m_operationObserver;
};

smtkCoordinateTransformView::smtkCoordinateTransformView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
  , m_p(new Internals(this, info))
{
#if 0
  if (auto* uiManager = info.get<qtUIManager*>())
  {
    const auto& managers = uiManager->managers();
    const auto& selection = managers.get<smtk::view::Selection::Ptr>();
    m_p->m_selectionObserver = selection->observers().insert(
      [this](const std::string& src, std::shared_ptr<smtk::view::Selection> const& sel) {
        m_p->selectionModified(src, sel);
      },
      std::numeric_limits<smtk::view::SelectionObservers::Priority>::lowest(),
      /* initialize immediately */ true,
      "update coordinate transform editor");
  }
#endif
}

smtkCoordinateTransformView::~smtkCoordinateTransformView()
{
  delete m_p;
}

bool smtkCoordinateTransformView::validateInformation(const smtk::view::Information& info)
{
  return qtOperationView::validateInformation(info);
}

qtBaseView* smtkCoordinateTransformView::createViewWidget(const smtk::view::Information& info)
{
  smtkCoordinateTransformView* view;
  if (!smtkCoordinateTransformView::validateInformation(info))
  {
    return nullptr;
  }
  view = new smtkCoordinateTransformView(info);
  view->buildUI();
  return view;
}

void smtkCoordinateTransformView::createWidget()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view)
  {
    return;
  }

  auto* parentlayout = static_cast<QVBoxLayout*>(this->parentWidget()->layout());

  // Delete any pre-existing widget
  if (this->Widget)
  {
    if (parentlayout)
    {
      parentlayout->removeWidget(this->Widget);
    }
    delete this->Widget;
  }

  // Create a new frame and lay it out
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName("smtkCoordinateTransformView");
  auto* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  m_p->m_editorLayout = new QHBoxLayout;
  this->updateUI();

  auto* wtmp = new QWidget;
  wtmp->setObjectName("GetRidOfMe");
  wtmp->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  m_p->setupUi(wtmp);
  m_p->m_setUp = true;
  layout->addWidget(wtmp);
  m_p->installActions();
  m_p->constructItemViews();

  QObject::connect(
    m_p->m_frameTree, SIGNAL(itemSelectionChanged()), this, SLOT(treeSelectionChanged()));
  this->treeSelectionChanged(); // Enable/disable actions to match current (empty) selection.
  this->parametersChanged();    // Enable/disable actions to match the current associations.
  m_p->installObserver();       // Observe operations in order to update the tree of landmarks.
}

void smtkCoordinateTransformView::onShowCategory()
{
  this->updateUI();
}

void smtkCoordinateTransformView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view || !this->Widget)
  {
    return;
  }

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
      if (optype == "edit properties")
      {
        //defName = optype;
        defName = "smtk::operation::CoordinateTransform";
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = m_p->m_currentOp->parameters();
}

void smtkCoordinateTransformView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  std::cout << "Item " << valItem->name() << " type " << valItem->type()
            << " changed; running op.\n";
  this->requestOperation(m_p->m_currentOp);
}

void smtkCoordinateTransformView::removeTransform()
{
  auto removeOption = m_p->m_currentOp->parameters()->findVoid("remove");
  removeOption->setIsEnabled(true);
  m_p->m_applyButton->setEnabled(false);
  m_p->m_actionRemoveTransform->setEnabled(false);
  this->requestOperation(m_p->m_currentOp);
}

void smtkCoordinateTransformView::applyTransform()
{
  auto removeOption = m_p->m_currentOp->parameters()->findVoid("remove");
  removeOption->setIsEnabled(false);
  m_p->m_applyButton->setEnabled(false);
  m_p->m_actionRemoveTransform->setEnabled(false);
  // Rather than observe the notes attached to the frames, just peek
  // at the widgets here to see if we need to save any remarks for
  // provenance before launching.
  m_p->checkLandmarkProvenance("from", m_p->m_fromFrameName->text().toStdString());
  m_p->checkLandmarkProvenance("to", m_p->m_toFrameName->text().toStdString());
  this->requestOperation(m_p->m_currentOp);
}

void smtkCoordinateTransformView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  this->uiManager()->operationManager()->launchers()(op);
  // op->operate();
}

void smtkCoordinateTransformView::treeSelectionChanged()
{
  // std::cout << "Tree selection changed to row " << m_p->m_frameTree->currentIndex().row() << ".\n";
  auto* item = m_p->m_frameTree->currentItem();
  bool frameSelected = (item && item->childCount() == 0);
  m_p->m_actionUseLandmarkAsFrom->setEnabled(frameSelected);
  m_p->m_actionUseLandmarkAsTo->setEnabled(frameSelected);
  vtkSMPropertyHelper(m_p->m_frameProxy, "Enabled").Set(frameSelected);
  m_p->m_frameProxy->UpdateVTKObjects();
  pqActiveObjects::instance().activeView()->render();
  smtk::resource::properties::CoordinateFrame frame;
  if (m_p->frameFromTree(dynamic_cast<FrameTreeItem*>(item), frame))
  {
    m_p->copyFrameToViewProxy(frame);
  }
}

void smtkCoordinateTransformView::togglePreview(bool showPreview)
{
  (void)showPreview;
  // vtkSMPropertyHelper(m_p->m_previewProxy, "Enabled").Set(showPreview);
  // std::cout << "Preview " << (showPreview ? "on" : "off") << "\n";
}

// The user activated a context menu to copy a landmark into the "from" frame:
void smtkCoordinateTransformView::useLandmarkAsFrom()
{
  auto* item = m_p->m_frameTree->currentItem();
  bool frameSelected = (item && item->childCount() == 0);
  if (frameSelected)
  {
    smtk::resource::properties::CoordinateFrame frame;
    auto* frameItem = dynamic_cast<FrameTreeItem*>(item);
    if (m_p->frameFromTree(frameItem, frame))
    {
      m_p->m_fromUsingLandmark = true;
      m_p->m_fromFrameName->setText(item->text(0));
      m_p->m_fromFrameName->setEnabled(false);
      m_p->m_fromFrameObject->setText(item->parent()->text(0));
      m_p->copyFrameToDestination(
        frame, m_p->m_currentOp->parameters()->findGroup("from"), m_p->m_fromItemWidget);
      // Also copy the landmark object and property name to the operation parameters
      // so they get recorded as provenance.
      m_p->copyLandmarkProvenance("from", frameItem->object(), frameItem->propertyName());
      this->parametersChanged();
    }
  }
}

// The user activated a context menu to copy a landmark into the "to" frame:
void smtkCoordinateTransformView::useLandmarkAsTo()
{
  auto* item = m_p->m_frameTree->currentItem();
  bool frameSelected = (item && item->childCount() == 0);
  if (frameSelected)
  {
    smtk::resource::properties::CoordinateFrame frame;
    auto* frameItem = dynamic_cast<FrameTreeItem*>(item);
    if (m_p->frameFromTree(frameItem, frame))
    {
      m_p->m_toUsingLandmark = true;
      m_p->m_toFrameName->setText(item->text(0));
      m_p->m_toFrameName->setEnabled(false);
      m_p->m_toFrameObject->setText(item->parent()->text(0));
      m_p->copyFrameToDestination(
        frame, m_p->m_currentOp->parameters()->findGroup("to"), m_p->m_toItemWidget);
      // Also copy the landmark object and property name to the operation parameters
      // so they get recorded as provenance.
      m_p->copyLandmarkProvenance("to", frameItem->object(), frameItem->propertyName());
      this->parametersChanged();
    }
  }
}

// The user edited the "from" coordinate frame; make sure we unset
// any provenance info from a landmark.
void smtkCoordinateTransformView::userEditedFromFrame()
{
  if (m_p->m_fromUsingLandmark)
  {
    m_p->m_fromFrameObject->setText("");
    m_p->m_fromFrameName->setText("");
    m_p->m_fromFrameName->setEnabled(true);
    m_p->m_fromUsingLandmark = false;
    m_p->disableLandmarkProvenance("from");
  }
  // Also enable/disable apply/remove buttons.
  this->parametersChanged();
}

// The user edited the "to" coordinate frame; make sure we unset
// any provenance info from a landmark.
void smtkCoordinateTransformView::userEditedToFrame()
{
  if (m_p->m_toUsingLandmark)
  {
    m_p->m_toFrameObject->setText("");
    m_p->m_toFrameName->setText("");
    m_p->m_toFrameName->setEnabled(true);
    m_p->m_toUsingLandmark = false;
    m_p->disableLandmarkProvenance("to");
  }
  // Also enable/disable apply/remove buttons.
  this->parametersChanged();
}

void smtkCoordinateTransformView::associationChanged()
{
  // TODO: Update preview proxy with components to preview
  this->parametersChanged();
}

void smtkCoordinateTransformView::parametersChanged()
{
  auto assoc = m_p->m_currentOp->parameters()->associations();
  bool haveObjects = assoc->numberOfSetValues() > 0;
  m_p->m_actionRemoveTransform->setEnabled(haveObjects);
  // TODO: Could check more parameters are valid before enabling:
  m_p->m_applyButton->setEnabled(haveObjects);
}

void smtkCoordinateTransformView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
