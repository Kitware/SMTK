//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKColorByWidget.h"
#include "ui_pqSMTKColorByWidget.h"

#include "pqComboBoxDomain.h"
#include "pqCoreUtilities.h"
#include "pqDataRepresentation.h"
#include "pqPropertyLinks.h"
#include "pqUndoStack.h"
#include "vtkPVXMLElement.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"

#include <QPointer>
#include <QSet>

#include <cstdlib>

//=============================================================================
class pqSMTKColorByWidget::PropertyLinksConnection : public pqPropertyLinksConnection
{
  typedef pqPropertyLinksConnection Superclass;

public:
  PropertyLinksConnection(
    QObject* qobject,
    const char* qproperty,
    const char* qsignal,
    vtkSMProxy* smproxy,
    vtkSMProperty* smproperty,
    int smindex,
    bool use_unchecked_modified_event,
    QObject* parentObject = nullptr)
    : Superclass(
        qobject,
        qproperty,
        qsignal,
        smproxy,
        smproperty,
        smindex,
        use_unchecked_modified_event,
        parentObject)
  {
  }
  ~PropertyLinksConnection() override = default;

protected:
  /// Called to update the ServerManager Property due to UI change.
  void setServerManagerValue(bool use_unchecked, const QVariant& value) override
  {
    Q_ASSERT(use_unchecked == false);
    Q_UNUSED(use_unchecked);

    BEGIN_UNDO_SET("Change color-by mode");
    vtkSMProxy* reprProxy = this->proxySM();
    vtkSMPropertyHelper(reprProxy, "ColorBy", true).Set(value.toString().toLocal8Bit().data());
    END_UNDO_SET();
  }

private:
  Q_DISABLE_COPY(PropertyLinksConnection)
};

//=============================================================================
class pqSMTKColorByWidget::pqInternals : public Ui::colorByWidget
{
  QString ColorByText;

public:
  pqPropertyLinks Links;
  QPointer<pqComboBoxDomain> Domain;
  QPointer<pqDataRepresentation> PQRepr;
  pqInternals() = default;

  bool setColorByText(const QString& text)
  {
    int idx = this->comboBox->findText(text);
    if (idx != -1)
    {
      bool prev = this->comboBox->blockSignals(true);
      this->comboBox->setCurrentIndex(idx);
      this->ColorByText = text;
      this->comboBox->blockSignals(prev);
    }
    return (idx != -1);
  }

  const QString& colorByText() const { return this->ColorByText; }
};

//-----------------------------------------------------------------------------
pqSMTKColorByWidget::pqSMTKColorByWidget(QWidget* _p)
  : Superclass(_p)
{
  this->Internal = new pqSMTKColorByWidget::pqInternals();
  this->Internal->setupUi(this);
  this->setToolTip("Choose colors used to draw SMTK components.");
  this->connect(
    this->Internal->comboBox,
    SIGNAL(currentIndexChanged(const QString&)),
    SLOT(comboBoxChanged(const QString&)));
}

//-----------------------------------------------------------------------------
pqSMTKColorByWidget::~pqSMTKColorByWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void pqSMTKColorByWidget::setRepresentation(pqDataRepresentation* display)
{
  if (this->Internal->PQRepr)
  {
    this->Internal->PQRepr->disconnect(this);
  }
  vtkSMProxy* proxy = display ? display->getProxy() : nullptr;
  this->setRepresentation(proxy);
  this->Internal->PQRepr = display;
  if (display)
  {
    display->connect(
      this, SIGNAL(colorByTextChanged(const QString&)), SLOT(renderViewEventually()));
  }
}

//-----------------------------------------------------------------------------
void pqSMTKColorByWidget::setRepresentation(vtkSMProxy* proxy)
{
  // break old links.
  this->Internal->Links.clear();
  delete this->Internal->Domain;
  bool prev = this->Internal->comboBox->blockSignals(true);
  this->Internal->comboBox->clear();
  vtkSMProperty* smproperty = proxy ? proxy->GetProperty("ColorBy") : nullptr;
  this->Internal->comboBox->setEnabled(smproperty != nullptr);
  if (!smproperty)
  {
    this->Internal->comboBox->addItem("Color Model By");
    this->Internal->comboBox->blockSignals(prev);
    Q_EMIT this->colorByFieldActive(true);
    return;
  }

  Q_EMIT this->colorByFieldActive(false);
  this->Internal->Domain = new pqComboBoxDomain(this->Internal->comboBox, smproperty);
  this->Internal->Links.addPropertyLink<PropertyLinksConnection>(
    this, "colorByText", SIGNAL(colorByTextChanged(const QString&)), proxy, smproperty);
  this->Internal->comboBox->blockSignals(prev);
}

//-----------------------------------------------------------------------------
void pqSMTKColorByWidget::setColorByText(const QString& text)
{
  this->Internal->setColorByText(text);
}

//-----------------------------------------------------------------------------
QString pqSMTKColorByWidget::colorByText() const
{
  return this->Internal->comboBox->isEnabled() ? this->Internal->colorByText() : QString();
}

//-----------------------------------------------------------------------------
void pqSMTKColorByWidget::comboBoxChanged(const QString& text)
{
  // NOTE: this method doesn't get called when
  // pqSMTKColorByWidget::setColorByText() is called.
  bool wasField = (this->Internal->colorByText() == "Field");
  this->Internal->setColorByText(text);
  Q_EMIT this->colorByTextChanged(text);
  bool fieldActive = (text == "Field");
  if (wasField ^ fieldActive)
  {
    Q_EMIT this->colorByFieldActive(fieldActive);
  }
}

//=============================================================================
pqSMTKColorByPropertyWidget::pqSMTKColorByPropertyWidget(vtkSMProxy* smProxy, QWidget* parentObject)
  : pqPropertyWidget(smProxy, parentObject)
{
  QVBoxLayout* layoutLocal = new QVBoxLayout;
  layoutLocal->setMargin(0);
  this->Widget = new pqSMTKColorByWidget(this);
  layoutLocal->addWidget(this->Widget);
  setLayout(layoutLocal);
  this->Widget->setRepresentation(smProxy);

  this->connect(
    this->Widget, SIGNAL(colorByTextChanged(const QString&)), SIGNAL(changeAvailable()));
}

//-----------------------------------------------------------------------------
pqSMTKColorByPropertyWidget::~pqSMTKColorByPropertyWidget() = default;
