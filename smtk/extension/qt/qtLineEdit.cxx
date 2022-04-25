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
   Module:    pqLineEdit.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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
#include "smtk/extension/qt/qtLineEdit.h"
using namespace smtk::extension;

//-----------------------------------------------------------------------------
qtLineEdit::qtLineEdit(QWidget* _parent)
  : Superclass(_parent)
{
  qtLineEdit::connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
  qtLineEdit::connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited()));
}

//-----------------------------------------------------------------------------
qtLineEdit::qtLineEdit(const QString& _contents, QWidget* _parent)
  : Superclass(_contents, _parent)
{
  qtLineEdit::connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
  qtLineEdit::connect(this, SIGNAL(textEdited(const QString&)), this, SLOT(onTextEdited()));
}

//-----------------------------------------------------------------------------
qtLineEdit::~qtLineEdit() = default;

//-----------------------------------------------------------------------------
void qtLineEdit::onTextEdited()
{
  this->EditingFinishedPending = true;
}

//-----------------------------------------------------------------------------
void qtLineEdit::onEditingFinished()
{
  if (this->EditingFinishedPending)
  {
    Q_EMIT this->textChangedAndEditingFinished();
    this->EditingFinishedPending = false;
  }
  if (this->ResetCursorPositionOnEditingFinished)
  {
    this->setCursorPosition(0);
  }
}

//-----------------------------------------------------------------------------
void qtLineEdit::setTextAndResetCursor(const QString& val)
{
  if (this->text() != val)
  {
    this->Superclass::setText(val);
    this->setCursorPosition(0);
  }
}

//-----------------------------------------------------------------------------
void qtLineEdit::triggerTextChangedAndEditingFinished()
{
  // Since we do not update this->EditingFinishedPending when the text is
  // changed programmatically, textChangedAndEditingFinished() wasn't getting
  // fired after setText() was called during test playback. To overcome that
  // issue, the playback manually calls this method.
  this->onTextEdited();
  this->onEditingFinished();
}
