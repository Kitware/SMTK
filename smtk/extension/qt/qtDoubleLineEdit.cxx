//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

/*=========================================================================

   Program: ParaView
   Module:  pqDoubleLineEdit.cxx

   Copyright (c) 2005-2018 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "smtk/extension/qt/qtDoubleLineEdit.h"

// Qt Includes.
#include <QDoubleValidator>
#include <QFocusEvent>
#include <QPointer>
#include <QTextStream>

#include <cassert>

using namespace smtk::extension;

//=============================================================================
namespace
{
//-----------------------------------------------------------------------------
QTextStream::RealNumberNotation toTextStreamNotation(qtDoubleLineEdit::RealNumberNotation notation)
{
  if (notation == qtDoubleLineEdit::FixedNotation)
  {
    return QTextStream::FixedNotation;
  }
  else if (notation == qtDoubleLineEdit::ScientificNotation)
  {
    return QTextStream::ScientificNotation;
  }
  else
  {
    return QTextStream::SmartNotation;
  }
}

//-----------------------------------------------------------------------------
using InstanceTrackerType = QList<qtDoubleLineEdit*>;
static InstanceTrackerType* InstanceTracker = nullptr;

//-----------------------------------------------------------------------------
void register_dle_instance(qtDoubleLineEdit* dle)
{
  if (InstanceTracker == nullptr)
  {
    InstanceTracker = new InstanceTrackerType();
  }
  InstanceTracker->push_back(dle);
}

void unregister_dle_instance(qtDoubleLineEdit* dle)
{
  assert(InstanceTracker != nullptr);
  InstanceTracker->removeOne(dle);
  if (InstanceTracker->size() == 0)
  {
    delete InstanceTracker;
    InstanceTracker = nullptr;
  }
}
}

//=============================================================================
class qtDoubleLineEdit::qtInternals
{
public:
  int Precision = 6;
  qtDoubleLineEdit::RealNumberNotation Notation = qtDoubleLineEdit::MixedNotation;
  bool UseGlobalPrecisionAndNotation = true;
  QPointer<QLineEdit> InactiveLineEdit = nullptr;

  bool useFullPrecision(const qtDoubleLineEdit* self) const { return self->hasFocus(); }

  void sync(qtDoubleLineEdit* self)
  {
    const auto real_precision =
      this->UseGlobalPrecisionAndNotation ? qtDoubleLineEdit::globalPrecision() : this->Precision;
    const auto real_notation =
      this->UseGlobalPrecisionAndNotation ? qtDoubleLineEdit::globalNotation() : this->Notation;

    bool changed = false;
    if (self->text().isEmpty())
    {
      this->InactiveLineEdit->setText("");
    }
    else
    {
      QString limited = qtDoubleLineEdit::formatDouble(
        self->text().toDouble(), toTextStreamNotation(real_notation), real_precision);

      changed = (limited != this->InactiveLineEdit->text());
      this->InactiveLineEdit->setText(limited);
    }
    auto pal = self->palette();
    this->InactiveLineEdit->setPalette(pal);
    if (changed & !this->useFullPrecision(self))
    {
      // ensures that if the low precision text changed and it was being shown on screen,
      // we repaint it.
      self->update();
    }
  }

  void renderSimplified(qtDoubleLineEdit* self)
  {
    if (this->InactiveLineEdit)
    {
      this->InactiveLineEdit->render(self, self->mapTo(self->window(), QPoint(0, 0)));
    }
  }
};

//=============================================================================
int qtDoubleLineEdit::GlobalPrecision = 6;
qtDoubleLineEdit::RealNumberNotation qtDoubleLineEdit::GlobalNotation =
  qtDoubleLineEdit::MixedNotation;

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::setGlobalPrecisionAndNotation(int precision, RealNumberNotation notation)
{
  bool modified = false;
  if (precision != qtDoubleLineEdit::GlobalPrecision)
  {
    qtDoubleLineEdit::GlobalPrecision = precision;
    modified = true;
  }

  if (qtDoubleLineEdit::GlobalNotation != notation)
  {
    qtDoubleLineEdit::GlobalNotation = notation;
    modified = true;
  }

  if (modified && InstanceTracker != nullptr)
  {
    for (const auto& instance : *InstanceTracker)
    {
      if (instance && instance->useGlobalPrecisionAndNotation())
      {
        instance->Internals->sync(instance);
      }
    }
  }
}

//-----------------------------------------------------------------------------
qtDoubleLineEdit::RealNumberNotation qtDoubleLineEdit::globalNotation()
{
  return qtDoubleLineEdit::GlobalNotation;
}

//-----------------------------------------------------------------------------
int qtDoubleLineEdit::globalPrecision()
{
  return qtDoubleLineEdit::GlobalPrecision;
}

//-----------------------------------------------------------------------------
qtDoubleLineEdit::qtDoubleLineEdit(QWidget* _parent)
  : Superclass(_parent)
  , Internals(new qtDoubleLineEdit::qtInternals())
{
  this->setValidator(new QDoubleValidator(this));
  register_dle_instance(this);

  auto& internals = (*this->Internals);
  internals.InactiveLineEdit = new QLineEdit();
  internals.InactiveLineEdit->hide();
  internals.sync(this);

  QObject::connect(
    this, &QLineEdit::textChanged, [this](const QString&) { this->Internals->sync(this); });
}

//-----------------------------------------------------------------------------
qtDoubleLineEdit::~qtDoubleLineEdit()
{
  unregister_dle_instance(this);
  auto& internals = (*this->Internals);
  delete internals.InactiveLineEdit;
  internals.InactiveLineEdit = nullptr;
}

//-----------------------------------------------------------------------------
qtDoubleLineEdit::RealNumberNotation qtDoubleLineEdit::notation() const
{
  auto& internals = (*this->Internals);
  return internals.Notation;
}

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::setNotation(qtDoubleLineEdit::RealNumberNotation _notation)
{
  auto& internals = (*this->Internals);
  if (internals.Notation != _notation)
  {
    internals.Notation = _notation;
    internals.sync(this);
  }
}

//-----------------------------------------------------------------------------
int qtDoubleLineEdit::precision() const
{
  auto& internals = (*this->Internals);
  return internals.Precision;
}

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::setPrecision(int _precision)
{
  auto& internals = (*this->Internals);
  if (internals.Precision != _precision)
  {
    internals.Precision = _precision;
    internals.sync(this);
  }
}

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::resizeEvent(QResizeEvent* evt)
{
  this->Superclass::resizeEvent(evt);
  auto& internals = (*this->Internals);
  internals.InactiveLineEdit->resize(this->size());
}

//-----------------------------------------------------------------------------
bool qtDoubleLineEdit::useGlobalPrecisionAndNotation() const
{
  auto& internals = (*this->Internals);
  return internals.UseGlobalPrecisionAndNotation;
}

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::setUseGlobalPrecisionAndNotation(bool value)
{
  auto& internals = (*this->Internals);
  if (internals.UseGlobalPrecisionAndNotation != value)
  {
    internals.UseGlobalPrecisionAndNotation = value;
    internals.sync(this);
  }
}

//-----------------------------------------------------------------------------
void qtDoubleLineEdit::paintEvent(QPaintEvent* evt)
{
  auto& internals = (*this->Internals);
  if (internals.useFullPrecision(this))
  {
    this->Superclass::paintEvent(evt);
  }
  else
  {
    internals.sync(this);
    internals.renderSimplified(this);
  }
}

//-----------------------------------------------------------------------------
QString qtDoubleLineEdit::simplifiedText() const
{
  auto& internals = (*this->Internals);
  return internals.InactiveLineEdit->text();
}

//-----------------------------------------------------------------------------
QString qtDoubleLineEdit::formatDouble(
  double value, QTextStream::RealNumberNotation notation, int precision)
{
  QString text;
  QTextStream converter(&text);
  converter.setRealNumberNotation(notation);
  converter.setRealNumberPrecision(precision);
  converter << value;

  return text;
}

//-----------------------------------------------------------------------------
QString qtDoubleLineEdit::formatDouble(
  double value, qtDoubleLineEdit::RealNumberNotation notation, int precision)
{
  return qtDoubleLineEdit::formatDouble(value, toTextStreamNotation(notation), precision);
}

//-----------------------------------------------------------------------------
QString qtDoubleLineEdit::formatDoubleUsingGlobalPrecisionAndNotation(double value)
{
  return qtDoubleLineEdit::formatDouble(
    value, qtDoubleLineEdit::globalNotation(), qtDoubleLineEdit::globalPrecision());
}
