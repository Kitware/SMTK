//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtDateTimeItem.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtOverlay.h"
#include "smtk/extension/qt/qtTimeZoneSelectWidget.h"

#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"
#include "smtk/common/DateTime.h"
#include "smtk/common/DateTimeZonePair.h"
#include "smtk/common/TimeZone.h"

#include <QAction>
#include <QCheckBox>
#include <QDate>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QSizePolicy>
#include <QTime>
#include <QToolButton>
#include <QWidget>

using namespace smtk::attribute;
using namespace smtk::extension;

//----------------------------------------------------------------------------
class qtDateTimeItem::qtDateTimeItemInternals
{
public:
  qtDateTimeItemInternals() {}
  ~qtDateTimeItemInternals() {}

  QPointer<QGridLayout> EntryLayout;
  QPointer<QLabel> theLabel;
  Qt::Orientation VectorItemOrient;

  // Components for time zone UI
  QDialog *TimeZoneDialog;
  QPushButton *TimeZoneDialogAcceptButton;
  qtTimeZoneSelectWidget *TimeZoneWidget;
  QToolButton *TimeZoneButton;
  QMenu *TimeZoneMenu;

  // for discrete items that with potential child widget
  // <Enum-Combo, child-layout >
  QMap<QWidget*, QPointer<QLayout> >ChildrenMap;
  QMap<QWidget*, int> ElementIndexMap;

  // for extensible items
  QMap<QToolButton*, QPair<QPointer<QLayout>, QPointer<QWidget> > > ExtensibleMap;
  QList<QToolButton*> MinusButtonIndices;
  QPointer<QToolButton> AddItemButton;
};

//----------------------------------------------------------------------------
qtDateTimeItem::qtDateTimeItem(
  smtk::attribute::DateTimeItemPtr item, QWidget* p, qtBaseView* bview,
   Qt::Orientation enVectorItemOrient) : qtItem(item, p, bview)
{
  this->Internals = new qtDateTimeItemInternals;

  this->Internals->TimeZoneDialog = NULL;
  this->Internals->TimeZoneDialogAcceptButton = NULL;
  this->Internals->TimeZoneWidget = NULL;
  this->Internals->TimeZoneButton = NULL;
  this->Internals->TimeZoneMenu = NULL;

  smtk::attribute::ConstDateTimeItemDefinitionPtr def =
    smtk::dynamic_pointer_cast<const smtk::attribute::DateTimeItemDefinition>(
      item->definition());
  if (def->useTimeZone())
    {
    this->Internals->TimeZoneDialog = new QDialog;
    this->Internals->TimeZoneDialog->setSizeGripEnabled(true);
    QVBoxLayout *dialogLayout = new QVBoxLayout();
    this->Internals->TimeZoneWidget = new qtTimeZoneSelectWidget;

    dialogLayout->addWidget(this->Internals->TimeZoneWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal,
      this->Internals->TimeZoneDialog);
    this->Internals->TimeZoneDialogAcceptButton = buttonBox->button(
      QDialogButtonBox::Ok);
    this->Internals->TimeZoneDialogAcceptButton->setEnabled(false);

    QObject::connect(
      this->Internals->TimeZoneWidget, SIGNAL(regionSelected(QString)),
      this, SLOT(onRegionSelected()));

    QObject::connect(
      buttonBox, SIGNAL(accepted()),
      this->Internals->TimeZoneDialog, SLOT(accept()));
    QObject::connect(
      buttonBox, SIGNAL(rejected()),
      this->Internals->TimeZoneDialog, SLOT(reject()));
    dialogLayout->addWidget(buttonBox);
    this->Internals->TimeZoneDialog->setLayout(dialogLayout);

    this->Internals->TimeZoneDialog->setWindowTitle("Select Time Zone");
    this->Internals->TimeZoneDialog->resize(800, 480);
    }

  this->Internals->VectorItemOrient = enVectorItemOrient;
  this->createWidget();
}

//----------------------------------------------------------------------------
qtDateTimeItem::~qtDateTimeItem()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void qtDateTimeItem::setLabelVisible(bool visible)
{
  this->Internals->theLabel->setVisible(visible);
}

//----------------------------------------------------------------------------
QWidget* qtDateTimeItem::createDateTimeWidget(int elementIdx)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<attribute::DateTimeItem>(this->getObject());
  smtk::attribute::ConstDateTimeItemDefinitionPtr def =
    dynamic_pointer_cast<const attribute::DateTimeItemDefinition>(item->definition());

  QDate qdate;
  QTime qtime;
  QDateTime qdatetime;
  QString timeZoneText("No TimeZone Selected");
  if (item->isSet())
    {
    ::smtk::common::DateTimeZonePair dtz = item->value(elementIdx);
    ::smtk::common::DateTime dt = dtz.dateTime();
    if (dt.isSet())
      {
      int year=-1, month=-1, day=-1, hour=-1, minute=-1, second=-1, msec=-1;
      if (dt.components(year, month, day, hour, minute, second, msec))
        {
        qdate.setDate(year, month, day);
        qdatetime.setDate(qdate);

        qtime.setHMS(hour, minute, second, msec);
        qdatetime.setTime(qtime);
        }
      }
    ::smtk::common::TimeZone tz = dtz.timeZone();
    if (tz.isSet())
      {
      if (tz.isUTC())
        {
        timeZoneText = "UTC";
        }
      else
        {
        QString tzRegion = QString::fromStdString(tz.region());
        this->Internals->TimeZoneWidget->setRegion(tzRegion);
        timeZoneText = tzRegion;
        }  // else (not UTC)
      }  // if (timezone set)
    }

  QFrame *frame = new QFrame(this->parentWidget());
  //frame->setStyleSheet("QFrame { background-color: yellow; }");

  QDateTimeEdit *dtEdit = new QDateTimeEdit(qdatetime, frame);
  this->Internals->ElementIndexMap.insert(dtEdit, elementIdx);
  dtEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  std::string format = def->displayFormat();
  if (!format.empty())
    {
    dtEdit->setDisplayFormat(QString::fromStdString(format));
    }
  dtEdit->setCalendarPopup(def->useCalendarPopup());
  QObject::connect(
    dtEdit, SIGNAL(dateTimeChanged(const QDateTime&)),
    this, SLOT(onDateTimeChanged(const QDateTime&)));

  // Use tooltip for min/max values
  // For now use unformatted json
  // Todo consider using display format
  QString tooltip;
  if (def->hasMinRange())
    {
    ::smtk::common::DateTimeZonePair minDtz = def->minRange();
    QString inclusive = def->minRangeInclusive() ? "Inclusive" : "Not Inclusive";
    QString minString = QString::fromStdString(minDtz.serialize());
    tooltip.append("Min (").append(inclusive).append("): ").append(minString);
    }
  if (def->hasMaxRange())
    {
    ::smtk::common::DateTimeZonePair maxDtz = def->maxRange();
    QString inclusive = def->maxRangeInclusive() ? "Inclusive" : "Not Inclusive";
    QString maxString = QString::fromStdString(maxDtz.serialize());
    if (!tooltip.isEmpty())
      {
      tooltip.append("\n");
      }
    tooltip.append("Max (").append(inclusive).append("): ").append(maxString);
    }

  if (!tooltip.isEmpty())
    {
    dtEdit->setToolTip(tooltip);
    }

  frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QHBoxLayout* layout = new QHBoxLayout(frame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(dtEdit);

  if (def->useTimeZone())
    {
    this->Internals->TimeZoneButton = new QToolButton(frame);
    this->Internals->TimeZoneButton->setSizePolicy(
      QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    this->Internals->TimeZoneButton->setText(timeZoneText);
    this->Internals->TimeZoneButton->setPopupMode(QToolButton::MenuButtonPopup);
    this->Internals->TimeZoneMenu = new QMenu(this->Internals->TimeZoneButton);
    this->Internals->TimeZoneMenu->addAction(
      "Unset TimeZone", this, SLOT(onTimeZoneUnset()));
    this->Internals->TimeZoneMenu->addAction(
      "UTC", this, SLOT(onTimeZoneUTC()));
    this->Internals->TimeZoneMenu->addAction(
      "Select Region...", this, SLOT(onTimeZoneRegion()));
    // Set element index on all actions
    foreach (QAction *action, this->Internals->TimeZoneMenu->actions())
      {
      action->setData(elementIdx);
      }

    this->Internals->TimeZoneButton->setMenu(this->Internals->TimeZoneMenu);
    //this->Internals->TimeZoneButton->setStyleSheet("background-color: white;");
    layout->addWidget(this->Internals->TimeZoneButton);
    }
  else
    {
    layout->addStretch();
    }

  layout->setAlignment(Qt::AlignLeft);
  this->updateBackground(dtEdit, item->isValid());
  return frame;
}

//----------------------------------------------------------------------------
void qtDateTimeItem::setOutputOptional(int)
{
}

//----------------------------------------------------------------------------
void qtDateTimeItem::updateItemData()
{
  this->updateUI();
  this->qtItem::updateItemData();
}

//----------------------------------------------------------------------------
void qtDateTimeItem::onChildWidgetSizeChanged()
{
}

//----------------------------------------------------------------------------
void qtDateTimeItem::onDateTimeChanged(const QDateTime& qdatetime)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<attribute::DateTimeItem>(this->getObject());
  QDateTimeEdit *dtEdit = dynamic_cast<QDateTimeEdit*>(this->sender());
  std::size_t element = this->Internals->ElementIndexMap.value(dtEdit);
  qDebug() << "onDateTimeChanged()" << qdatetime << element;

  // Convert QDateTime to smtk::attribute::DateTime
  QDate qdate = qdatetime.date();
  int year=-1, month=-1, day=-1;
  qdate.getDate(&year, &month, &day);

  QTime qtime = qdatetime.time();
  int hour=-1, minute=-1, second=-1, msec=-1;
  hour = qtime.hour();
  minute = qtime.minute();
  second = qtime.second();
  msec = qtime.msec();

  smtk::common::DateTimeZonePair dtz = item->value(element);
  smtk::common::DateTime dt = dtz.dateTime();
  dt.setComponents(year, month, day, hour, minute, second, msec);
  dtz.setDateTime(dt);
  bool valid = item->setValue(element, dtz);
  this->updateBackground(dtEdit, valid);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::onRegionSelected()
{
  this->Internals->TimeZoneDialogAcceptButton->setEnabled(true);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::onTimeZoneUnset()
{
  QAction *action = dynamic_cast<QAction*>(this->sender());
  std::size_t element = action->data().toInt();

  this->Internals->TimeZoneButton->setText("No TimeZoneSelected");
  this->setTimeZone(element, "");

  this->updateTimeZoneMenu(action);
}
//----------------------------------------------------------------------------
void qtDateTimeItem::onTimeZoneUTC()
{
  QAction *action = dynamic_cast<QAction*>(this->sender());
  std::size_t element = action->data().toInt();

  this->Internals->TimeZoneButton->setText("UTC");
  this->setTimeZoneToUTC(element);
  this->updateTimeZoneMenu(action);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::onTimeZoneRegion()
{
  QAction *action = dynamic_cast<QAction*>(this->sender());
  std::size_t element = action->data().toInt();

  if (this->Internals->TimeZoneDialog->exec() == QDialog::Accepted)
    {
    // Update UI
    QString regionId = this->Internals->TimeZoneWidget->selectedRegion();
    qDebug() << "Accepted" << regionId;
    this->Internals->TimeZoneButton->setText(regionId);

    // Update item
    this->setTimeZone(element, regionId);
    }
  this->updateTimeZoneMenu(action);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::createWidget()
{
  smtk::attribute::ItemPtr dataObj = this->getObject();
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->clearChildWidgets();
  this->updateItemData();
}

//----------------------------------------------------------------------------
void qtDateTimeItem::loadInputValues()
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());
  if(!item)
    {
    return;
    }

  int n = static_cast<int>(item->numberOfValues());
//   if(item->isExtensible())
//     {
//     if(!this->Internals->AddItemButton)
//       {
//       QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//       this->Internals->AddItemButton = new QToolButton(this->Widget);
//       QString iconName(":/icons/attribute/plus.png");
//       this->Internals->AddItemButton->setText("Add New Value");
//       this->Internals->AddItemButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

// //      this->Internals->AddItemButton->setFixedSize(QSize(12, 12));
//       this->Internals->AddItemButton->setIcon(QIcon(iconName));
//       this->Internals->AddItemButton->setSizePolicy(sizeFixedPolicy);
//       connect(this->Internals->AddItemButton, SIGNAL(clicked()),
//         this, SLOT(onAddNewValue()));
//       this->Internals->EntryLayout->addWidget(this->Internals->AddItemButton, 0, 1);
//       }
//     }

  for(int i = 0; i < n; i++)
    {
    this->addInputEditor(i);
    }
}

//----------------------------------------------------------------------------
void qtDateTimeItem::updateUI()
{
  smtk::attribute::DateTimeItemPtr dataObj =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());
  if(!dataObj || !this->passAdvancedCheck() || (this->baseView() &&
    !this->baseView()->uiManager()->passItemCategoryCheck(
      dataObj->definition())))
    {
    return;
    }

  this->Widget = new QFrame(this->parentWidget());
  this->Internals->EntryLayout = new QGridLayout(this->Widget);
  this->Internals->EntryLayout->setMargin(0);
  this->Internals->EntryLayout->setSpacing(0);
  this->Internals->EntryLayout->setAlignment( Qt::AlignLeft | Qt::AlignTop );

  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QHBoxLayout* labelLayout = new QHBoxLayout();
  labelLayout->setMargin(0);
  labelLayout->setSpacing(0);
  labelLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  int padding = 0;
  if(dataObj->isOptional())
    {
    QCheckBox* optionalCheck = new QCheckBox(this->parentWidget());
    optionalCheck->setChecked(dataObj->isEnabled());
    optionalCheck->setText(" ");
    optionalCheck->setSizePolicy(sizeFixedPolicy);
    padding = optionalCheck->iconSize().width() + 3; // 6 is for layout spacing
    QObject::connect(optionalCheck, SIGNAL(stateChanged(int)),
      this, SLOT(setOutputOptional(int)));
    labelLayout->addWidget(optionalCheck);
    }
  const DateTimeItemDefinition *itemDef =
    dynamic_cast<const DateTimeItemDefinition*>(dataObj->definition().get());

  QString labelText;
  if(!dataObj->label().empty())
    {
    labelText = dataObj->label().c_str();
    }
  else
    {
    labelText = dataObj->name().c_str();
    }
  QLabel* label = new QLabel(labelText, this->Widget);
  label->setSizePolicy(sizeFixedPolicy);
  if(this->baseView())
    {
    label->setFixedWidth(this->baseView()->fixedLabelWidth() - padding);
    }
  label->setWordWrap(true);
  label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

//  qtOverlayFilter *filter = new qtOverlayFilter(this);
//  label->installEventFilter(filter);

  // add in BriefDescription as tooltip if available
  const std::string strBriefDescription = itemDef->briefDescription();
  if(!strBriefDescription.empty())
    {
    label->setToolTip(strBriefDescription.c_str());
    }

  if(itemDef->advanceLevel() && this->baseView())
    {
    label->setFont(this->baseView()->uiManager()->advancedFont());
    }
  labelLayout->addWidget(label);
  this->Internals->theLabel = label;

  this->loadInputValues();

  // we need this layout so that for items with conditionan children,
  // the label will line up at Top-left against the chilren's widgets.
//  QVBoxLayout* vTLlayout = new QVBoxLayout;
//  vTLlayout->setMargin(0);
//  vTLlayout->setSpacing(0);
//  vTLlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
//  vTLlayout->addLayout(labelLayout);
  this->Internals->EntryLayout->addLayout(labelLayout, 0, 0);
//  layout->addWidget(this->Internals->EntryFrame, 0, 1);
  if(this->parentWidget() && this->parentWidget()->layout())
    {
    this->parentWidget()->layout()->addWidget(this->Widget);
    }
  if(dataObj->isOptional())
    {
    this->setOutputOptional(dataObj->isEnabled() ? 1 : 0);
    }
}

//----------------------------------------------------------------------------
void qtDateTimeItem::addInputEditor(int i)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());
  if(!item)
    {
    return;
    }

  int n = static_cast<int>(item->numberOfValues());
  if (!n)
    {
    return;
    }

  QBoxLayout* childLayout = NULL;
  childLayout = new QVBoxLayout;
  childLayout->setContentsMargins(12, 3, 3, 0);
  childLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

  QWidget* editBox = this->createDateTimeWidget(i);
  if(!editBox)
    {
    return;
    }

  const DateTimeItemDefinition *itemDef =
    dynamic_cast<const DateTimeItemDefinition*>(item->definition().get());
  QSizePolicy sizeFixedPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  QBoxLayout* editorLayout = new QHBoxLayout;
  editorLayout->setMargin(0);
  editorLayout->setSpacing(3);
  if(item->isExtensible())
    {
    QToolButton* minusButton = new QToolButton(this->Widget);
    QString iconName(":/icons/attribute/minus.png");
    minusButton->setFixedSize(QSize(12, 12));
    minusButton->setIcon(QIcon(iconName));
    minusButton->setSizePolicy(sizeFixedPolicy);
    minusButton->setToolTip("Remove value");
    editorLayout->addWidget(minusButton);
    connect(minusButton, SIGNAL(clicked()),
      this, SLOT(onRemoveValue()));
    QPair<QPointer<QLayout>, QPointer<QWidget> > pair;
    pair.first = editorLayout;
    pair.second = editBox;
    this->Internals->ExtensibleMap[minusButton] = pair;
    this->Internals->MinusButtonIndices.push_back(minusButton);
    }

  if(n!=1 && itemDef->hasValueLabels())
    {
    std::string componentLabel = itemDef->valueLabel(i);
    if(!componentLabel.empty())
      {
      // acbauer -- this should probably be improved to look nicer
      QString labelText = componentLabel.c_str();
      QLabel* label = new QLabel(labelText, editBox);
      label->setSizePolicy(sizeFixedPolicy);
      editorLayout->addWidget(label);
      }
    }
  editorLayout->addWidget(editBox);

  // always going vertical for discrete and extensible items
  if(this->Internals->VectorItemOrient == Qt::Vertical || item->isExtensible())
    {
    int row = 2*i;
    // The "Add New Value" button is in first row, so take that into account
    row = item->isExtensible() ? row+1 : row;
    this->Internals->EntryLayout->addLayout(editorLayout, row, 1);

    // there could be conditional children, so we need another layout
    // so that the combobox will stay TOP-left when there are multiple
    // combo boxes.
    if(childLayout)
      {
      this->Internals->EntryLayout->addLayout(childLayout, row+1, 0, 1, 2);
      }
    }
  else // going horizontal
    {
    this->Internals->EntryLayout->addLayout(editorLayout, 0, i+1);
    }

  this->Internals->ChildrenMap[editBox] = childLayout;
  this->updateExtensibleState();
}

//----------------------------------------------------------------------------
void qtDateTimeItem::updateExtensibleState()
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());
  if(!item || !item->isExtensible())
    {
    return;
    }
  bool maxReached = (item->maxNumberOfValues() > 0) &&
    (item->maxNumberOfValues() == item->numberOfValues());
  this->Internals->AddItemButton->setEnabled(!maxReached);

  bool minReached = (item->numberOfRequiredValues() > 0) &&
    (item->numberOfRequiredValues() == item->numberOfValues());
  foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
    {
    tButton->setEnabled(!minReached);
    }
}

//----------------------------------------------------------------------------
void qtDateTimeItem::clearChildWidgets()
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());
  if(!item)
    {
    return;
    }

//   if(item->isExtensible())
//     {
//     //clear mapping
//     foreach(QToolButton* tButton, this->Internals->ExtensibleMap.keys())
//       {
// // will delete later from this->Internals->ChildrenMap
// //      delete this->Internals->ExtensibleMap.value(tButton).second;
//       delete this->Internals->ExtensibleMap.value(tButton).first;
//       delete tButton;
//       }
//     this->Internals->ExtensibleMap.clear();
//     this->Internals->MinusButtonIndices.clear();
//     }

  // foreach(QWidget* cwidget, this->Internals->ChildrenMap.keys())
  //   {
  //   QLayout* childLayout = this->Internals->ChildrenMap.value(cwidget);
  //   if(childLayout)
  //     {
  //     QLayoutItem *child;
  //     while ((child = childLayout->takeAt(0)) != 0)
  //       {
  //       delete child;
  //       }
  //     delete childLayout;
  //     }
  //   delete cwidget;
  //   }
  // this->Internals->ChildrenMap.clear();
}

//----------------------------------------------------------------------------
void qtDateTimeItem::updateBackground(QDateTimeEdit *dtEdit, bool valid)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<attribute::DateTimeItem>(this->getObject());
  std::size_t element = this->Internals->ElementIndexMap.value(dtEdit);
  smtk::common::DateTimeZonePair dtz = item->value(element);

  // Set background coloring
  QColor color;
  if (!valid)
    {
    color = this->uiManager()->invalidValueColor();
    }
  else if (item->isUsingDefault(element))
    {
    //qDebug() << element <<  " -- Default DateTimeZonePair" << def->hasDefault();
    color = this->uiManager()->defaultValueColor();
    }
  else
    {
    //qDebug() << element <<  " -- Normal DateTimeZonePair";
    color = Qt::white;
    }
  // QPalette does not work with QDateTimeEdit (?)
  // so using stylesheet instead
  QString ss = QString("background-color:rgb(%1,%2,%3)").
    arg(color.red()).arg(color.green()).arg(color.blue());
  dtEdit->setStyleSheet(ss);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::updateTimeZoneMenu(QAction *selectedAction)
{
  foreach (QAction *action, this->Internals->TimeZoneMenu->actions())
    {
    bool enabled = action != selectedAction;
    action->setEnabled(enabled);
    }
}

//----------------------------------------------------------------------------
void qtDateTimeItem::setTimeZone(std::size_t element, const QString& region)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());

  ::smtk::common::DateTimeZonePair tzPair = item->value(element);
  ::smtk::common::TimeZone tz = tzPair.timeZone();
  tz.setRegion(region.toStdString());
  tzPair.setTimeZone(tz);
  item->setValue(element, tzPair);
}

//----------------------------------------------------------------------------
void qtDateTimeItem::setTimeZoneToUTC(std::size_t element)
{
  smtk::attribute::DateTimeItemPtr item =
    dynamic_pointer_cast<DateTimeItem>(this->getObject());

  smtk::common::DateTimeZonePair tzPair = item->value(element);
  smtk::common::TimeZone tz = tzPair.timeZone();
  tz.setUTC();
  tzPair.setTimeZone(tz);
  item->setValue(element, tzPair);
}

//----------------------------------------------------------------------------
