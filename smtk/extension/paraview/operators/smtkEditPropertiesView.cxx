//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkEditPropertiesView.h"
#include "smtk/operation/operators/EditProperties.h"

#include "smtk/extension/paraview/operators/ui_smtkEditPropertiesView.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtAttributeItemInfo.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/properties/CoordinateFrame.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"
#include "smtk/view/SelectionObserver.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPresetDialog.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"
#include "smtk/extension/paraview/widgets/pqSMTKCoordinateFrameItemWidget.h"
#include "smtk/extension/qt/qtReferenceItemEditor.h"

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
#include <QTimer>
#include <QVBoxLayout>

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
constexpr auto coordinateFrameTextValue = "<coordinate frame>";
} // namespace

class smtkEditPropertiesView::Internals : public Ui::smtkEditPropertiesView
{
public:
  Internals(const std::shared_ptr<smtk::operation::Operation>& op)
    : m_currentOp(std::dynamic_pointer_cast<smtk::operation::EditProperties>(op))
  {
    m_attributeValueTable = nullptr;
    if (m_currentOp)
    {
      auto typeItem = m_currentOp->parameters()->findInt("type");
      auto coordinateFrameGroup = std::dynamic_pointer_cast<smtk::attribute::GroupItem>(
        typeItem->find("Coordinate Frame", smtk::attribute::SearchStyle::IMMEDIATE));
      /*
      m_widgetCoordinateFrameGroupItem =
        std::make_shared<smtk::attribute::GroupItem>(*coordinateFrameGroup);
        */
      m_widgetCoordinateFrameGroupItem = coordinateFrameGroup;
      // auto parentItem = m_widgetCoordinateFrameGroupItem->findAs<ReferenceItem>("Parent");
      // m_coordinateSystemParent = parentItem; // std::make_shared<smtk::attribute::ReferenceItem>(*parentItem);
    }
    else
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create an edit operation.");
    }
  }

  ~Internals() = default;

  void selectionModified(
    const std::string& /*source*/,
    const std::shared_ptr<smtk::view::Selection>& selection)
  {
    std::set<std::shared_ptr<smtk::resource::PersistentObject>> tmp;
    selection->currentSelectionByValue(tmp, "selected", false);
    if (!tmp.empty())
    {
      m_objects = tmp;
      // If m_setUp is false, wait for event processing to resume
      // before updating the table as the widgets have not been
      // constructed yet.
      if (m_setUp)
      {
        this->updateProperties();
      }
      else
      {
        QTimer::singleShot(0, [this]() { this->updateProperties(); });
      }
    }
  }

  void updateProperties()
  {
    struct Key
    {
      std::string name;
      std::string type;

      bool operator<(const Key& other) const
      {
        return this->name < other.name || (this->name == other.name && this->type < other.type);
      }
    };
    using Entry = struct
    {
      QVariant value;
      std::size_t count;
    };
    if (!m_setUp || !m_attributeValueTable)
    {
      return;
    }
    if (m_objects.size() == 1)
    {
      m_editorLabel->setText(
        QString("Editing %1").arg(QString::fromStdString(m_objects.begin()->get()->name())));
    }
    else
    {
      m_editorLabel->setText(QString("Editing %1 objects").arg(m_objects.size()));
    }
    m_attributeValueTable->clearContents();
    std::map<Key, Entry> props;
    std::size_t expected = 0;
    for (const auto& object : m_objects)
    {
      if (object)
      {
        ++expected;
        const auto& objectProps = object->properties();
        {
          const auto& intProps = objectProps.get<long>();
          for (const auto& propName : intProps.keys())
          {
            Key key{ propName, "integer" };
            auto it = props.find(key);
            if (it == props.end())
            {
              props[key] = Entry{ QVariant::fromValue(intProps.at(propName)), 1 };
            }
            else
            {
              ++props[key].count;
              QVariant val = QVariant::fromValue(intProps.at(propName));
              if (val != it->second.value)
              {
                props[key].value = QString("<multiple values>");
              }
            }
          }
        }
        {
          const auto& fpProps = objectProps.get<double>();
          for (const auto& propName : fpProps.keys())
          {
            Key key{ propName, "floating-point" };
            auto it = props.find(key);
            if (it == props.end())
            {
              props[key] = Entry{ QVariant::fromValue(fpProps.at(propName)), 1 };
            }
            else
            {
              ++props[key].count;
              QVariant val = QVariant::fromValue(fpProps.at(propName));
              if (val != it->second.value)
              {
                props[key].value = QString("<multiple values>");
              }
            }
          }
        }
        {
          const auto& fpProps = objectProps.get<CoordinateFrame>();
          for (const auto& propName : fpProps.keys())
          {
            Key key{ propName, "coordinate-frame" };
            auto it = props.find(key);
            if (it == props.end())
            {
              props[key] = Entry{ QString(coordinateFrameTextValue), 1 };
              // const CoordinateFrame& frame = fpProps.at(propName);
            }
            else
            {
              ++props[key].count;
              props[key].value = QString(coordinateFrameTextValue);
            }
          }
        }
        {
          const auto& strProps = objectProps.get<std::string>();
          for (const auto& propName : strProps.keys())
          {
            Key key{ propName, "string" };
            auto it = props.find(key);
            if (it == props.end())
            {
              props[key] = Entry{ QString::fromStdString(strProps.at(propName)), 1 };
            }
            else
            {
              ++props[key].count;
              QVariant val = QString::fromStdString(strProps.at(propName));
              if (val != it->second.value)
              {
                props[key].value = QString("<multiple values>");
              }
            }
          }
        }
      }
    }
    m_attributeValueTable->setRowCount(static_cast<int>(props.size()));
    int row = 0;
    for (auto& prop : props)
    {
      m_attributeValueTable->setItem(
        row, 0, new QTableWidgetItem(QString::fromStdString(prop.first.name)));
      m_attributeValueTable->setItem(
        row, 1, new QTableWidgetItem(QString::fromStdString(prop.first.type)));
      if (prop.second.count != expected)
      {
        prop.second.value = QString("<partial>");
      }
      m_attributeValueTable->setItem(row, 2, new QTableWidgetItem(prop.second.value.toString()));
      ++row;
    }
  }

  void copySelectedRowToEditor(::smtkEditPropertiesView* view)
  {
    auto index = m_attributeValueTable->currentIndex();
    index = index.siblingAtColumn(0);
    if (!index.isValid())
    {
      // No row is selected. This happens when the operation completes because
      // the table is regenerated with any newly-added properties and the selection
      // is reset. In that case, we want to leave the editor just as it was so
      // users can continue to edit the same property.
      return;
    }

    m_attributeNameEdit->setText(index.data().toString());
    QString typeString = index.siblingAtColumn(1).data().toString();
    bool showValue = true;

    if (typeString == "string")
    {
      m_attributeTypeCombo->setCurrentIndex(0);
      m_attributeValuesEdit->setValidator(nullptr);
    }
    else if (typeString == "floating-point")
    {
      m_attributeTypeCombo->setCurrentIndex(1);
      m_attributeValuesEdit->setValidator(new QDoubleValidator());
    }
    else if (typeString == "integer")
    {
      m_attributeTypeCombo->setCurrentIndex(2);
      m_attributeValuesEdit->setValidator(new QIntValidator());
    }
    else if (typeString == "coordinate-frame")
    {
      // Copy first object's CoordinateFrame to the editor widget.
      // It is assumed that if m_objects has multiple objects, they all
      // have the same value of CoordinateFrame with the specific selected name.
      // Either way, applying this operation will make it consistent.
      if (!m_objects.empty())
      {
        const auto& object = *m_objects.begin();
        if (object)
        {
          const std::string propName = index.data().toString().toStdString();
          const auto& objectProps = object->properties();
          if (objectProps.contains<CoordinateFrame>(propName))
          {
            const auto& f = objectProps.at<CoordinateFrame>(propName);
            updateWidgetCoordinateFrame(f, view);
          }
        }
      }
      // This triggers an event call to propertyTypeChanged(), so we
      // want to make sure m_widgetCoordinateFrame is updated before that happens.
      m_attributeTypeCombo->setCurrentIndex(3);
      showValue = false;
    }

    m_propertyValuesLabel->setVisible(showValue);
    m_attributeValuesEdit->setVisible(showValue);

    if (showValue)
    {
      m_attributeValuesEdit->setText(index.siblingAtColumn(2).data().toString());
    }
  }

  void editAttribute(const std::string& attributeName, bool erase = false)
  {
    // Check that inputs are valid
    QString attributeValues = m_attributeValuesEdit->text();
    QVariant val = attributeValues;
    if (attributeName.empty())
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "You must specify an attribute name.");
      return;
    }
    auto op = m_currentOp;
    if (!op)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create an edit operation.");
      return;
    }
    auto assoc = op->parameters()->associations();
    assoc->setValues(m_objects.begin(), m_objects.end(), 0);
    op->parameters()->findString("name")->setValue(attributeName);
    int attributeType =
      m_attributeTypeCombo->currentIndex(); // 0 = string, 1 = fp, 2 = int, 3 = coordinate frame
    auto typeItem = op->parameters()->findInt("type");
    typeItem->setValue(attributeType);
    auto removeItem = op->parameters()->find("remove");

    if (erase)
    {
      removeItem->setIsEnabled(true);

      // We still need to set values here or the operation will be
      // unable to proceed because the parameters will be considered
      // invalid.
      switch (attributeType)
      {
        default:
        case 0:
        {
          auto stringItem = std::dynamic_pointer_cast<smtk::attribute::StringItem>(
            typeItem->find("string value", smtk::attribute::SearchStyle::IMMEDIATE));
          stringItem->setValue("");
          break;
        }
        case 1:
        {
          auto doubleItem = std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(
            typeItem->find("float value", smtk::attribute::SearchStyle::IMMEDIATE));
          doubleItem->setValue(0.0);
          break;
        }
        case 2:
        {
          auto intItem = std::dynamic_pointer_cast<smtk::attribute::IntItem>(
            typeItem->find("integer value", smtk::attribute::SearchStyle::IMMEDIATE));
          intItem->setValue(0);
          break;
        }
      }
    }
    else
    {
      removeItem->setIsEnabled(false);

      switch (attributeType)
      {
        default:
        case 0:
        {
          auto stringItem = std::dynamic_pointer_cast<smtk::attribute::StringItem>(
            typeItem->find("string value", smtk::attribute::SearchStyle::IMMEDIATE));
          stringItem->setValue(val.toString().toStdString());
          break;
        }
        case 1:
        {
          auto doubleItem = std::dynamic_pointer_cast<smtk::attribute::DoubleItem>(
            typeItem->find("float value", smtk::attribute::SearchStyle::IMMEDIATE));
          doubleItem->setValue(val.toDouble());
          break;
        }
        case 2:
        {
          auto intItem = std::dynamic_pointer_cast<smtk::attribute::IntItem>(
            typeItem->find("integer value", smtk::attribute::SearchStyle::IMMEDIATE));
          intItem->setValue(val.toInt());
          break;
        }
      }
    }
    auto result = op->operate();
    (void)result; // TODO: Verify
    this->updateProperties();
  }

  bool updateWidgetCoordinateFrame(const CoordinateFrame& frame, ::smtkEditPropertiesView* view)
  {
    m_widgetCoordinateFrameGroupItem->setNumberOfGroups(1);
    auto originItem = m_widgetCoordinateFrameGroupItem->findAs<DoubleItem>("Origin");
    auto xAxisItem = m_widgetCoordinateFrameGroupItem->findAs<DoubleItem>("XAxis");
    auto yAxisItem = m_widgetCoordinateFrameGroupItem->findAs<DoubleItem>("YAxis");
    auto zAxisItem = m_widgetCoordinateFrameGroupItem->findAs<DoubleItem>("ZAxis");
    auto parentItem = m_widgetCoordinateFrameGroupItem->findAs<ReferenceItem>("Parent");
    if (
      originItem && xAxisItem && yAxisItem && zAxisItem &&
      parentItem) // && m_coordinateSystemParent)
    {
      originItem->setValues(frame.origin.begin(), frame.origin.end());
      xAxisItem->setValues(frame.xAxis.begin(), frame.xAxis.end());
      yAxisItem->setValues(frame.yAxis.begin(), frame.yAxis.end());
      zAxisItem->setValues(frame.zAxis.begin(), frame.zAxis.end());
      if (!frame.parent.isNull())
      {
        parentItem->setIsEnabled(false);
        parentItem->unset();
        auto rsrcMgr = view->uiManager()->resourceManager();
        if (rsrcMgr)
        {
          rsrcMgr->visit([&parentItem, &frame](smtk::resource::Resource& rsrc) {
            auto comp = rsrc.find(frame.parent);
            if (comp)
            {
              parentItem->setIsEnabled(true);
              parentItem->setValue(comp);
              return smtk::common::Processing::STOP;
            }
            return smtk::common::Processing::CONTINUE;
          });
        }
        if (!parentItem->isEnabled())
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "Coordinate frame parent component (id "
              << frame.parent << ") could not be found (perhaps a resource has not been loaded?).");
        }
      }
      else
      {
        // No parent... disable the optional parent item.
        parentItem->setIsEnabled(false);
        parentItem->unset();
      }
      if (m_frameWidget)
      {
        m_frameWidget->updateWidgetFromItem();
      }
      return true;
    }
    return false;
  }

  bool m_setUp{ false };
  QPointer<QHBoxLayout> m_editorLayout;
  std::shared_ptr<smtk::operation::EditProperties> m_currentOp;
  smtk::view::SelectionObservers::Key m_selectionObserver;
  std::set<std::shared_ptr<smtk::resource::PersistentObject>> m_objects;
  pqSMTKCoordinateFrameItemWidget* m_frameWidget{ nullptr };
  smtk::attribute::GroupItemPtr m_widgetCoordinateFrameGroupItem{ nullptr };
  QVBoxLayout* m_frameWidgetContainerLayout{ nullptr };
};

smtkEditPropertiesView::smtkEditPropertiesView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
  , m_p(new Internals(info.get<smtk::operation::Operation::Ptr>()))
{
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
      "update freeform attribute editor");
  }
}

smtkEditPropertiesView::~smtkEditPropertiesView()
{
  delete m_p;
}

bool smtkEditPropertiesView::displayItem(const smtk::attribute::ItemPtr& item) const
{
  if (item && item->name() == "colors")
  {
    return false;
  }
  return this->qtBaseAttributeView::displayItem(item);
}

bool smtkEditPropertiesView::validateInformation(const smtk::view::Information& info)
{
  return qtOperationView::validateInformation(info);
}

qtBaseView* smtkEditPropertiesView::createViewWidget(const smtk::view::Information& info)
{
  smtkEditPropertiesView* view;
  if (!smtkEditPropertiesView::validateInformation(info))
  {
    return nullptr;
  }
  view = new smtkEditPropertiesView(info);
  view->buildUI();
  return view;
}

void smtkEditPropertiesView::createWidget()
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
  this->Widget->setObjectName("smtkEditPropertiesView");
  auto* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  m_p->m_editorLayout = new QHBoxLayout;
  this->updateUI();

  auto* wtmp = new QWidget;
  wtmp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  m_p->setupUi(wtmp);
  m_p->m_frameWidgetContainerLayout = new QVBoxLayout(m_p->m_frameWidgetContainer);
  m_p->m_setUp = true;
  layout->addWidget(wtmp);

  /*
  QObject::connect(m_p->m_attributeValueTable,
    SIGNAL(itemSelectionChanged()),
    this,
    SLOT(tableSelectionChanged()));
    */
  QObject::connect(
    m_p->m_attributeValueTable,
    SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),
    this,
    SLOT(tableSelectionChanged()));
  QObject::connect(m_p->m_addButton, SIGNAL(released()), this, SLOT(addOrReplaceAttribute()));
  QObject::connect(m_p->m_removeButton, SIGNAL(released()), this, SLOT(removeSelectedAttribute()));

  // Check if the CoordinateFrameWidget needs to be shown, if the current attribute type changes.
  QObject::connect(
    m_p->m_attributeTypeCombo,
    SIGNAL(currentIndexChanged(int)),
    this,
    SLOT(propertyTypeChanged(int)));

  // Show help when the info button is clicked.
  // QObject::connect(m_p->InfoBtn, SIGNAL(released()), this, SLOT(onInfo()));
}

void smtkEditPropertiesView::onShowCategory()
{
  this->updateUI();
}

void smtkEditPropertiesView::updateUI()
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
        defName = "smtk::operation::EditProperties";
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

void smtkEditPropertiesView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  op->operate();
}

void smtkEditPropertiesView::tableSelectionChanged()
{
  m_p->copySelectedRowToEditor(this);
}

void smtkEditPropertiesView::addOrReplaceAttribute()
{
  std::string attributeName = m_p->m_attributeNameEdit->text().toStdString();
  m_p->editAttribute(attributeName, false);
}

void smtkEditPropertiesView::removeSelectedAttribute()
{
  const auto& indices = m_p->m_attributeValueTable->selectionModel()->selectedIndexes();
  for (const auto& index : indices)
  {
    if (index.column() == 0)
    {
      std::string attributeName = index.data().toString().toStdString();
      m_p->editAttribute(attributeName, true);
    }
  }
}

void smtkEditPropertiesView::propertyTypeChanged(int index)
{
  if (index < 3)
  {
    m_p->m_propertyValuesLabel->setVisible(true);
    m_p->m_attributeValuesEdit->setVisible(true);
    m_p->m_attributeValuesEdit->setText("");
    if (m_p->m_frameWidget)
    {
      m_p->m_frameWidget->markForDeletion();
      m_p->m_frameWidget = nullptr;
    }
    switch (index)
    {
      case 1: // Floating point Input
        m_p->m_attributeValuesEdit->setValidator(new QDoubleValidator(this));
        break;
      case 2: // Integer Input
        m_p->m_attributeValuesEdit->setValidator(new QIntValidator(this));
        break;
      default:
        // No validation needed
        m_p->m_attributeValuesEdit->setValidator(nullptr);
    }
  }
  else
  {
    if (m_p->m_frameWidget)
    {
      m_p->m_frameWidget->markForDeletion();
    }
    m_p->m_propertyValuesLabel->setVisible(false);
    m_p->m_attributeValuesEdit->setVisible(false);
    qtAttributeItemInfo info(
      m_p->m_widgetCoordinateFrameGroupItem,
      this->m_viewInfo.configuration()->details(),
      m_p->m_frameWidgetContainer,
      this);
    m_p->m_frameWidget = dynamic_cast<pqSMTKCoordinateFrameItemWidget*>(
      pqSMTKCoordinateFrameItemWidget::createCoordinateFrameItemWidget(info));
    m_p->m_frameWidgetContainerLayout->addWidget(m_p->m_frameWidget->widget());
  }
}

void smtkEditPropertiesView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  /*
  std::cout << "Item " << valItem->name() << " type " << valItem->type()
            << " changed; running op.\n";
   */
  this->requestOperation(m_p->m_currentOp);
}

void smtkEditPropertiesView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
