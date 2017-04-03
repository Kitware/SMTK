//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/paraview/operators/smtkAssignColorsView.h"
#include "smtk/extension/paraview/operators/ui_smtkAssignColorsParameters.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/bridge/polygon/qt/pqArcWidgetManager.h"
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"
#include "smtk/bridge/polygon/qt/pqSplitEdgeWidget.h"
#include "smtk/common/View.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtModelOperationWidget.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/model/Operator.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqPresetDialog.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSettings.h"

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

#define SMTK_DEFAULT_PALETTE "KAAMS"

using namespace smtk::extension;

//----------------------------------------------------------------------------
class smtkAssignColorsViewInternals : public Ui::AssignColorsParameters
{
public:
  smtkAssignColorsViewInternals()
    {
    }

  ~smtkAssignColorsViewInternals()
    {
    if (CurrentAtt)
      delete CurrentAtt;
    }

  qtAttribute* createAttUI(
    smtk::attribute::AttributePtr att,
    QWidget* pw,
    qtBaseView* view
    )
  {
    if (att && att->numberOfItems() > 0)
      {
      qtAttribute* attInstance = new qtAttribute(att, pw, view);
      if(attInstance && attInstance->widget())
        {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("assignColorsEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
        }
      return attInstance;
      }
    return NULL;
  }

  bool paletteFromDialog(QList<QColor>& colors, const QString& paletteName)
    {
    this->PaletteChooser->setCurrentPreset(paletteName.toUtf8().constData());
    const Json::Value& preset(this->PaletteChooser->currentPreset());
    if (!preset.isMember("IndexedColors"))
      {
      return false;
      }
    const Json::Value& jsonColors(preset["IndexedColors"]);
    Json::ArrayIndex numColors = jsonColors.size() / 3;
    for (Json::ArrayIndex cc = 0; cc < numColors; ++cc)
      {
      std::ostringstream colorStr;
      colorStr << "#";
      for (int cm = 0; cm < 3; ++cm)
        {
        int val = static_cast<int>(jsonColors[3 * cc + cm].asDouble() * 255.0);
        colorStr << std::setfill('0') << std::setw(2) << std::hex << val;
        }
      //colorStr << "ff"; // Alpha is not provided for indexed colors, nor interpreted by QColor constructor properly. Grrr.
      colors.append(QColor(colorStr.str().c_str()));
      }
    return true;
    }

  bool updatePaletteIcon(const std::string& name)
    {
    QList<QColor> palette;
    if (this->paletteFromDialog(palette, name.c_str()))
      {
      /*
      int ww = this->ApplyDefaultPaletteBtn->width() - 2;
      int hh = 0.75 * this->ApplyDefaultPaletteBtn->height();
      */
      int ww = 64;
      int hh = 16;
      this->ApplyDefaultPaletteBtn->setIconSize(QSize(ww, hh));

      this->ApplyDefaultPaletteBtn->setIcon(
        smtkAssignColorsView::renderPaletteSwatch(
          palette, ww, hh));
      return true;
      }
    return false;
    }

  QPointer<qtAttribute> CurrentAtt;
  QPointer<QHBoxLayout> EditorLayout;

  QPointer<pqPresetDialog> PaletteChooser;
  smtk::weak_ptr<smtk::model::Operator> CurrentOp;
};

//----------------------------------------------------------------------------
smtkAssignColorsView::smtkAssignColorsView(const ViewInfo &info)
  : qtBaseView(info)
{
  this->Internals = new smtkAssignColorsViewInternals;
}

//----------------------------------------------------------------------------
smtkAssignColorsView::~smtkAssignColorsView()
{
  delete this->Internals;
}

bool smtkAssignColorsView::displayItem(smtk::attribute::ItemPtr item)
{
  if (item && item->name() == "colors")
    {
    return false;
    }
  return this->qtBaseView::displayItem(item);
}

//----------------------------------------------------------------------------
qtBaseView* smtkAssignColorsView::createViewWidget(const ViewInfo &info)
{
  smtkAssignColorsView *view = new smtkAssignColorsView(info);
  view->buildUI();
  return view;
}


//----------------------------------------------------------------------------
QIcon smtkAssignColorsView::renderColorSwatch(const QColor& color, int radius)
{
  if (radius < 10)
    {
    radius = 10;
    }

  QPixmap pix(radius, radius);
  pix.fill(QColor(0, 0, 0, 0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setBrush(QBrush(color));
  painter.drawEllipse(1, 1, radius - 2, radius - 2);
  painter.end();
  return QIcon(pix);
}

//----------------------------------------------------------------------------
QIcon smtkAssignColorsView::renderPaletteSwatch(
  const QList<QColor>& colors,
  int width,
  int radius)
{
  if (radius < 10)
    {
    radius = 10;
    }

  QPixmap pix(width, radius);
  pix.fill(QColor(0, 0, 0, 0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  double delta = (width - radius - 2.0) / colors.size();
  for (int cs = 0; cs < colors.size(); ++cs)
    {
    painter.setBrush(QBrush(colors.at(cs)));
    painter.drawEllipse(1 + delta * cs, 1, radius - 2, radius - 2);
    }
  painter.end();
  return QIcon(pix);
}

//----------------------------------------------------------------------------
QIcon smtkAssignColorsView::renderInvalidSwatch(int radius)
{
  if (radius < 10)
    {
    radius = 10;
    }

  QPixmap pix(radius, radius);
  pix.fill(QColor(0, 0, 0, 0));

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setBrush(QBrush(Qt::white));
  painter.drawEllipse(1, 1, radius - 2, radius - 2);
  painter.setPen(Qt::red);
  double rm1 = radius - 1.0;
  double dd = rm1 / sqrt(2.0) / 2.0;
  double ctr = rm1 / 2.0;
  QLineF slash(ctr - dd, ctr + dd, ctr + dd, ctr - dd);
  painter.drawLines(&slash, 1);
  painter.setBrush(QBrush(Qt::transparent));
  painter.setPen(Qt::black);
  painter.drawEllipse(1, 1, radius - 2, radius - 2);
  painter.end();
  return QIcon(pix);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::attributeModified()
{
  // Always enable the apply button here.
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::createWidget( )
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view)
    {
    return;
    }

  QVBoxLayout* parentlayout = static_cast<QVBoxLayout*> (
    this->parentWidget()->layout());

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
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  this->Internals->EditorLayout = new QHBoxLayout;
  this->updateAttributeData();

  QWidget* wtmp = new QWidget;
  this->Internals->setupUi(wtmp);
  layout->addWidget(wtmp);
  // Fetch user preferences
  pqSettings* settings = pqApplicationCore::instance()->settings();
  QString defaultPalette = settings->value("ModelBuilder/Operators/AssignColors/defaultPalette", SMTK_DEFAULT_PALETTE).toString();
  QColor defColor = Qt::white;
  QColor defaultColor = settings->value("ModelBuilder/Operators/AssignColors/defaultColor", defColor).value<QColor>();

  // Set up a preset dialog; display only categorical (indexed) colormaps.
  this->Internals->PaletteChooser = new pqPresetDialog(nullptr, pqPresetDialog::SHOW_INDEXED_COLORS_ONLY);
  this->Internals->PaletteChooser->setCustomizableLoadColors(false);
  this->Internals->PaletteChooser->setCustomizableLoadOpacities(false);
  this->Internals->PaletteChooser->setCustomizableUsePresetRange(false);
  this->Internals->PaletteChooser->setCustomizableLoadAnnotations(true);

  this->Internals->ApplyDefaultColorBtn->setIcon(
    smtkAssignColorsView::renderColorSwatch(
      defaultColor,
      0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  this->Internals->ApplyDefaultPaletteBtn->setText(defaultPalette);
  std::string defaultPaletteStr(defaultPalette.toUtf8().constData());
  this->Internals->updatePaletteIcon(defaultPaletteStr);

  this->Internals->RemoveColorBtn->setIcon(
    smtkAssignColorsView::renderInvalidSwatch(0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  // Signals and slots related to palette mode (this->Internals->PaletteChooser):
  QObject::connect( // When asked, apply the colormap specified by the user preference.
    this->Internals->ApplyDefaultPaletteBtn, SIGNAL(released()),
    this, SLOT(applyDefaultPalette()));
  QObject::connect( // Allow the user to choose a new preference.
    this->Internals->ChoosePaletteBtn, SIGNAL(released()),
    this->Internals->PaletteChooser, SLOT(show()));
  QObject::connect( // When the user has chosen a preference, remember and apply it.
    this->Internals->PaletteChooser, SIGNAL(applyPreset(const Json::Value&)),
    this, SLOT(setDefaultPaletteAndApply(const Json::Value&)));

  // Signals and slots related to single-color mode:
  QObject::connect( // When asked, apply the colormap specified by the user preference.
    this->Internals->ApplyDefaultColorBtn, SIGNAL(released()),
    this, SLOT(applyDefaultColor()));
  QObject::connect( // Allow the user to choose a new preference.
    this->Internals->ChooseColorBtn, SIGNAL(released()),
    this, SLOT(chooseDefaultColorAndApply()));

  QObject::connect( // When asked, invalidate colors on the associated entities.
    this->Internals->RemoveColorBtn, SIGNAL(released()),
    this, SLOT(removeColors()));

  QObject::disconnect(this->uiManager()->activeModelView());
  QObject::connect(this->uiManager()->activeModelView(),
    SIGNAL(operationCancelled(const smtk::model::OperatorPtr&)),
    this, SLOT(cancelOperation(const smtk::model::OperatorPtr&)));
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::updateAttributeData()
{
  smtk::common::ViewPtr view = this->getObject();
  if (!view || !this->Widget)
    {
    return;
    }

  if(this->Internals->CurrentAtt)
    {
    delete this->Internals->CurrentAtt;
    }

  int i = view->details().findChild("AttributeTypes");
  if (i < 0)
    {
    return;
    }
  smtk::common::View::Component& comp = view->details().child(i);
  std::string defName;
  for (std::size_t ci = 0; ci < comp.numberOfChildren(); ++ci)
    {
    smtk::common::View::Component &attComp = comp.child(ci);
    //std::cout << "  component " << attComp.name() << "\n";
    if (attComp.name() != "Att")
      {
      continue;
      }
    std::string optype;
    if (attComp.attribute("Type", optype) && !optype.empty())
      {
      //std::cout << "    component type " << optype << "\n";
      if (optype == "assign colors")
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

  smtk::model::OperatorPtr assignColorsOp =
    this->uiManager()->activeModelView()->
    operatorsWidget()->existingOperator(defName);
  this->Internals->CurrentOp = assignColorsOp;

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = assignColorsOp->specification();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::requestOperation(const smtk::model::OperatorPtr& op)
{
  if (!op || !op->specification())
    {
    return;
    }
  this->uiManager()->activeModelView()->requestOperation(op, false);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if ( !op || !this->Widget || !this->Internals->CurrentAtt )
    {
    return;
    }
  // Reset widgets here
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  //std::cout << "Item " << valItem->name() << " type " << valItem->type() << " changed\n";
  this->requestOperation(this->Internals->CurrentOp.lock());
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::chooseDefaultColorAndApply()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  QColor nextDefault =
    QColorDialog::getColor(
      settings->value("ModelBuilder/Operators/AssignColors/defaultColor", QColor(Qt::white)).
      value<QColor>(),
      nullptr, "Choose a default color for \"assign colors\"", QColorDialog::DontUseNativeDialog);
  settings->setValue("ModelBuilder/Operators/AssignColors/defaultColor", nextDefault);
  this->Internals->ApplyDefaultColorBtn->setIcon(
    smtkAssignColorsView::renderColorSwatch(
      nextDefault,
      0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  this->applyDefaultColor();
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::applyDefaultColor()
{
  auto op = this->Internals->CurrentOp.lock();
  if (!this->Internals->CurrentAtt || !this->Widget || !op)
    {
    return;
    }

  pqSettings* settings = pqApplicationCore::instance()->settings();
  QColor color = settings->value(
    "ModelBuilder/Operators/AssignColors/defaultColor", QColor(Qt::white)).value<QColor>();

  //std::cout << "Apply default color\n";

  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setNumberOfValues(1);
  colorsItem->setValue(0, color.name().toUtf8().constData());

  this->requestOperation(op);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::setDefaultPaletteAndApply(const Json::Value& preset)
{
  std::string name(preset["Name"].asString().c_str());
  //std::cout << "Change default palette to \"" << name << "\"\n";
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue("ModelBuilder/Operators/AssignColors/defaultPalette", name.c_str());
  this->Internals->PaletteChooser->hide();
  this->Internals->ApplyDefaultPaletteBtn->setText(QString::fromUtf8(name.c_str()));
  this->Internals->updatePaletteIcon(name);

  this->applyDefaultPalette();
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::applyDefaultPalette()
{
  auto op = this->Internals->CurrentOp.lock();
  if (!this->Internals->CurrentAtt || !this->Widget || !op)
    {
    return;
    }

  pqSettings* settings = pqApplicationCore::instance()->settings();
  QString paletteName = settings->value("ModelBuilder/Operators/AssignColors/defaultPalette", SMTK_DEFAULT_PALETTE).toString();
  //std::cout << "Apply default \"" << paletteName.toUtf8().constData()<< "\"\n";

  QList<QColor> palette;
  if (!this->Internals->paletteFromDialog(palette, paletteName))
    {
    std::cerr << "Error, no color palette named \"" << paletteName.toUtf8().constData() << "\"\n";
    return;
    }

  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setNumberOfValues(palette.size());
  for (int pp = 0; pp < palette.size(); ++pp)
    {
    colorsItem->setValue(pp, palette.at(pp).name().toUtf8().constData());
    }

  this->requestOperation(op);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::removeColors()
{
  auto op = this->Internals->CurrentOp.lock();
  if (!this->Internals->CurrentAtt || !op)
    {
    return;
    }

  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setNumberOfValues(0);
  this->requestOperation(op);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::clearSelection()
{
  this->uiManager()->activeModelView()->clearSelection();
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::showAdvanceLevelOverlay(bool show)
{
  this->qtBaseView::showAdvanceLevelOverlay(show);
}

//----------------------------------------------------------------------------
void smtkAssignColorsView::requestModelEntityAssociation()
{
  this->updateAttributeData();
}
