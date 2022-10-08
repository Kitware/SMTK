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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/view/Configuration.h"

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

class smtkAssignColorsViewInternals : public Ui::AssignColorsParameters
{
public:
  smtkAssignColorsViewInternals() = default;

  ~smtkAssignColorsViewInternals()
  {
    if (CurrentAtt)
      delete CurrentAtt;
  }

  qtAttribute* createAttUI(smtk::attribute::AttributePtr att, QWidget* pw, qtBaseView* view)
  {
    if (att && att->numberOfItems() > 0)
    {
      smtk::view::Configuration::Component comp;
      qtAttribute* attInstance = new qtAttribute(att, comp, pw, view);
      // attInstance->setUseSelectionManager(view->useSelectionManager());
      if (attInstance && attInstance->widget())
      {
        //Without any additional info lets use a basic layout with model associations
        // if any exists
        attInstance->createBasicLayout(true);
        attInstance->widget()->setObjectName("assignColorsEditor");
        QVBoxLayout* parentlayout = static_cast<QVBoxLayout*>(pw->layout());
        parentlayout->insertWidget(0, attInstance->widget());
      }
      return attInstance;
    }
    return nullptr;
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
        smtkAssignColorsView::renderPaletteSwatch(palette, ww, hh));
      return true;
    }
    return false;
  }

  QPointer<qtAttribute> CurrentAtt;
  QPointer<QHBoxLayout> EditorLayout;

  QPointer<pqPresetDialog> PaletteChooser;
  smtk::shared_ptr<smtk::operation::Operation> CurrentOp;
};

smtkAssignColorsView::smtkAssignColorsView(const smtk::view::Information& info)
  : qtBaseAttributeView(info)
{
  this->Internals = new smtkAssignColorsViewInternals;
  this->Internals->CurrentOp = info.get<smtk::shared_ptr<smtk::operation::Operation>>();
}

smtkAssignColorsView::~smtkAssignColorsView()
{
  delete this->Internals;
}

bool smtkAssignColorsView::displayItem(smtk::attribute::ItemPtr item) const
{
  if (item && item->name() == "colors")
  {
    return false;
  }
  return this->qtBaseAttributeView::displayItem(item);
}

qtBaseView* smtkAssignColorsView::createViewWidget(const smtk::view::Information& info)
{
  if (qtOperationView::validateInformation(info))
  {
    auto* view = new smtkAssignColorsView(info);
    view->buildUI();
    return view;
  }
  return nullptr; // Information is not suitable for this View
}

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

QIcon smtkAssignColorsView::renderPaletteSwatch(const QList<QColor>& colors, int width, int radius)
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

void smtkAssignColorsView::attributeModified()
{
  // Always enable the apply button here.
}

// Properly create and initialize the PalatteChooser Diaglog Widget
void smtkAssignColorsView::prepPaletteChooser()
{
  // Set up a preset dialog; display only categorical (indexed) colormaps.
  this->Internals->PaletteChooser =
    new pqPresetDialog(this->parentWidget(), pqPresetDialog::SHOW_INDEXED_COLORS_ONLY);
  this->Internals->PaletteChooser->setCustomizableLoadColors(false);
  this->Internals->PaletteChooser->setCustomizableLoadOpacities(false);
  this->Internals->PaletteChooser->setCustomizableUsePresetRange(false);
  this->Internals->PaletteChooser->setCustomizableLoadAnnotations(true);
  // Signals and slots related to palette mode (this->Internals->PaletteChooser):
  QObject::connect( // Allow the user to choose a new preference.
    this->Internals->ChoosePaletteBtn,
    &QToolButton::released,
    this->Internals->PaletteChooser,
    &pqPresetDialog::show);
  QObject::connect( // When the user has chosen a preference, remember and apply it.
    this->Internals->PaletteChooser,
    &pqPresetDialog::applyPreset,
    this,
    &smtkAssignColorsView::setDefaultPaletteAndApply);
}

void smtkAssignColorsView::createWidget()
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

  // Create a new frame and lay it out
  this->Widget = new QFrame(this->parentWidget());
  this->Widget->setObjectName("assignColorsView");
  QVBoxLayout* layout = new QVBoxLayout(this->Widget);
  layout->setMargin(0);
  this->Widget->setLayout(layout);
  this->Widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  this->Internals->EditorLayout = new QHBoxLayout;
  this->updateUI();

  QWidget* wtmp = new QWidget;
  this->Internals->setupUi(wtmp);
  layout->addWidget(wtmp);
  // Fetch user preferences
  pqSettings* settings = pqApplicationCore::instance()->settings();
  QString defaultPalette =
    settings->value("ModelBuilder/Operations/AssignColors/defaultPalette", SMTK_DEFAULT_PALETTE)
      .toString();
  QColor defColor = Qt::white;
  QColor defaultColor =
    settings->value("ModelBuilder/Operations/AssignColors/defaultColor", defColor).value<QColor>();

  this->prepPaletteChooser();
  this->Internals->ApplyDefaultColorBtn->setIcon(smtkAssignColorsView::renderColorSwatch(
    defaultColor, 0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  this->Internals->ApplyDefaultPaletteBtn->setText(defaultPalette);
  std::string defaultPaletteStr(defaultPalette.toUtf8().constData());
  this->Internals->updatePaletteIcon(defaultPaletteStr);

  this->Internals->RemoveColorBtn->setIcon(smtkAssignColorsView::renderInvalidSwatch(
    0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  QObject::connect( // When asked, apply the colormap specified by the user preference.
    this->Internals->ApplyDefaultPaletteBtn,
    SIGNAL(released()),
    this,
    SLOT(applyDefaultPalette()));

  // Signals and slots related to single-color mode:
  QObject::connect( // When asked, apply the colormap specified by the user preference.
    this->Internals->ApplyDefaultColorBtn,
    SIGNAL(released()),
    this,
    SLOT(applyDefaultColor()));
  QObject::connect( // Allow the user to choose a new preference.
    this->Internals->ChooseColorBtn,
    SIGNAL(released()),
    this,
    SLOT(chooseDefaultColorAndApply()));

  // Signals and slots related to opacity mode:
  QObject::connect( // When asked, apply the opacity specified by the slider.
    this->Internals->OpacitySlider,
    SIGNAL(valueChanged(int)),
    this,
    SLOT(processOpacitySlider(int)));

  QObject::connect( // When asked, apply the opacity specified by the slider.
    this->Internals->OpacityValue,
    SIGNAL(valueChanged(double)),
    this,
    SLOT(processOpacityValue(double)));

  QObject::connect( // When asked, invalidate colors on the associated entities.
    this->Internals->RemoveColorBtn,
    SIGNAL(released()),
    this,
    SLOT(removeColors()));

  // Show help when the info button is clicked.
  QObject::connect(this->Internals->InfoBtn, SIGNAL(released()), this, SLOT(onInfo()));
}

void smtkAssignColorsView::onShowCategory()
{
  this->updateUI();
}

void smtkAssignColorsView::updateUI()
{
  smtk::view::ConfigurationPtr view = this->configuration();
  if (!view || !this->Widget)
  {
    return;
  }

  if (this->Internals->CurrentAtt)
  {
    delete this->Internals->CurrentAtt;
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
        //defName = optype;
        defName = "smtk::operation::AssignColors";
        break;
      }
    }
  }
  if (defName.empty())
  {
    return;
  }

  // expecting only 1 instance of the op?
  smtk::attribute::AttributePtr att = this->Internals->CurrentOp->parameters();
  this->Internals->CurrentAtt = this->Internals->createAttUI(att, this->Widget, this);
}

void smtkAssignColorsView::requestOperation(const smtk::operation::OperationPtr& op)
{
  if (!op || !op->parameters())
  {
    return;
  }
  op->operate();
}

void smtkAssignColorsView::valueChanged(smtk::attribute::ItemPtr valItem)
{
  (void)valItem;
  //std::cout << "Item " << valItem->name() << " type " << valItem->type() << " changed\n";
  this->requestOperation(this->Internals->CurrentOp);
}

void smtkAssignColorsView::chooseDefaultColorAndApply()
{
  pqSettings* settings = pqApplicationCore::instance()->settings();
  QColor nextDefault = QColorDialog::getColor(
    settings->value("ModelBuilder/Operations/AssignColors/defaultColor", QColor(Qt::white))
      .value<QColor>(),
    nullptr,
    "Choose a default color for \"assign colors\"",
    QColorDialog::DontUseNativeDialog);
  settings->setValue("ModelBuilder/Operations/AssignColors/defaultColor", nextDefault);
  this->Internals->ApplyDefaultColorBtn->setIcon(smtkAssignColorsView::renderColorSwatch(
    nextDefault, 0.75 * this->Internals->ApplyDefaultColorBtn->height()));

  this->applyDefaultColor();
}

void smtkAssignColorsView::applyDefaultColor()
{
  if (!this->Internals->CurrentAtt || !this->Widget || !this->Internals->CurrentOp)
  {
    return;
  }

  pqSettings* settings = pqApplicationCore::instance()->settings();
  QColor color =
    settings->value("ModelBuilder/Operations/AssignColors/defaultColor", QColor(Qt::white))
      .value<QColor>();

  //std::cout << "Apply default color\n";

  auto opacityItem = this->Internals->CurrentAtt->attribute()->findDouble("opacity");
  opacityItem->setIsEnabled(false);

  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setIsEnabled(true);
  colorsItem->setNumberOfValues(1);
  colorsItem->setValue(0, color.name().toUtf8().constData());

  this->requestOperation(this->Internals->CurrentOp);
}

void smtkAssignColorsView::setDefaultPaletteAndApply()
{
  const Json::Value& preset = this->Internals->PaletteChooser->currentPreset();
  std::string name(preset["Name"].asString());
  //std::cerr << "Change default palette to \"" << name << "\"" << std::endl;
  pqSettings* settings = pqApplicationCore::instance()->settings();
  settings->setValue("ModelBuilder/Operations/AssignColors/defaultPalette", name.c_str());

  // pqPresetDialog has a custom event filter in ParaView, disabling everything except
  // the render window when opened.
  // If we don't 'close', the UI does not seem to respond after you set the default palette.
  // The close slot cleans up the custom event filter which blocks UI events.
  // Also makes the dialog testable with recorded xml tests.
  this->Internals->PaletteChooser->close();

  this->Internals->ApplyDefaultPaletteBtn->setText(QString::fromUtf8(name.c_str()));
  this->Internals->updatePaletteIcon(name);

  this->applyDefaultPalette();
}

void smtkAssignColorsView::applyDefaultPalette()
{
  if (!this->Internals->CurrentAtt || !this->Widget || !this->Internals->CurrentOp)
  {
    return;
  }

  pqSettings* settings = pqApplicationCore::instance()->settings();
  QString paletteName =
    settings->value("ModelBuilder/Operations/AssignColors/defaultPalette", SMTK_DEFAULT_PALETTE)
      .toString();
  //std::cout << "Apply default \"" << paletteName.toUtf8().constData()<< "\"\n";

  QList<QColor> palette;
  if (!this->Internals->paletteFromDialog(palette, paletteName))
  {
    std::cerr << "Error, no color palette named \"" << paletteName.toUtf8().constData() << "\"\n";
    return;
  }

  auto opacityItem = this->Internals->CurrentAtt->attribute()->findDouble("opacity");
  opacityItem->setIsEnabled(false);
  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setIsEnabled(true);
  colorsItem->setNumberOfValues(palette.size());
  for (int pp = 0; pp < palette.size(); ++pp)
  {
    colorsItem->setValue(pp, palette.at(pp).name().toUtf8().constData());
  }

  this->requestOperation(this->Internals->CurrentOp);
}

void smtkAssignColorsView::processOpacitySlider(int val)
{
  // Tell the spinbox what the new value is
  double opacity = val / 255.0;
  this->Internals->OpacityValue->setValue(opacity);
}

void smtkAssignColorsView::processOpacityValue(double val)
{
  auto opacityItem = this->Internals->CurrentAtt->attribute()->findDouble("opacity");
  opacityItem->setIsEnabled(true);
  opacityItem->setValue(val);

  // Disable color assignment
  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setIsEnabled(false);

  this->requestOperation(this->Internals->CurrentOp);
  // Lets make sure the opacity slider reflects this new value
  this->Internals->OpacitySlider->blockSignals(true);
  int sval = 255 * val;
  this->Internals->OpacitySlider->setValue(sval);
  this->Internals->OpacitySlider->blockSignals(false);
}

void smtkAssignColorsView::removeColors()
{
  if (!this->Internals->CurrentAtt || !this->Internals->CurrentOp)
  {
    return;
  }

  auto opacityItem = this->Internals->CurrentAtt->attribute()->findDouble("opacity");
  opacityItem->setIsEnabled(false);

  smtk::attribute::StringItem::Ptr colorsItem =
    this->Internals->CurrentAtt->attribute()->findString("colors");
  colorsItem->setIsEnabled(true);
  colorsItem->setNumberOfValues(0);
  this->requestOperation(this->Internals->CurrentOp);
}

void smtkAssignColorsView::setInfoToBeDisplayed()
{
  m_infoDialog->displayInfo(this->configuration());
}
