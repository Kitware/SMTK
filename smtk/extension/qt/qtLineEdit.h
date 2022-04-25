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
   Module:    pqLineEdit.h

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
#ifndef qtLineEdit_h
#define qtLineEdit_h

#include "smtk/extension/qt/Exports.h"
#include <QLineEdit>

namespace smtk
{
namespace extension
{
/**
* qtLineEdit is a specialization of QLineEdit which provide a new property
* 'text2'. When the text on the line widget is set using this 'text2' property
* (or using setTextAndResetCursor()), the after the set, the cursor position
* is reset to 0.
*
* Additional, this provides a true editingFinished() signal under the name of
* textChangedAndEditingFinished(). Unlike QLineEdit::editingFinished() which
* gets fired whenever the widget looses focus irrespective of if the text
* actually was edited, textChangedAndEditingFinished() is fired only when the
* text was changed as well.
*
* To enable/disable whether the cursor position is reset to 0 after
* textChangedAndEditingFinished() if fired, use the
* resetCursorPositionOnEditingFinished property (default: true).
 *
 * This is from ParaView's pqLineEdit class.
*/
class SMTKQTEXT_EXPORT qtLineEdit : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString text2 READ text WRITE setTextAndResetCursor)
  Q_PROPERTY(bool resetCursorPositionOnEditingFinished READ resetCursorPositionOnEditingFinished
               WRITE setResetCursorPositionOnEditingFinished)

  typedef QLineEdit Superclass;

public:
  qtLineEdit(QWidget* parent = nullptr);
  qtLineEdit(const QString& contents, QWidget* parent = nullptr);

  ~qtLineEdit() override;

  /**
  * To enable/disable whether the cursor position is reset to 0 after
  * editingFinished() is fired, use the
  * resetCursorPositionOnEditingFinished property (default: true).
  */
  bool resetCursorPositionOnEditingFinished() const
  {
    return this->ResetCursorPositionOnEditingFinished;
  }

Q_SIGNALS:
  /**
  * Unlike QLineEdit::editingFinished() which
  * gets fired whenever the widget looses focus irrespective of if the text
  * actually was edited, textChangedAndEditingFinished() is fired only when the
  * text was changed as well.
  */
  void textChangedAndEditingFinished();

public Q_SLOTS:
  /**
  * Same as QLineEdit::setText() except that it reset the cursor position to
  * 0.  This is useful with the qtLineEdit is used for showing numbers were
  * the digits on the left are more significant on the right.
  */
  void setTextAndResetCursor(const QString& text);

  /**
  * To enable/disable whether the cursor position is reset to 0 after
  * editingFinished() is fired, use the
  * resetCursorPositionOnEditingFinished property (default: true).
  */
  void setResetCursorPositionOnEditingFinished(bool val)
  {
    this->ResetCursorPositionOnEditingFinished = val;
  }

private Q_SLOTS:
  void onTextEdited();
  void onEditingFinished();

protected:
  friend class qtLineEditEventPlayer;
  /**
  * this is called by qtLineEditEventPlayer during event playback to ensure
  * that the textChangedAndEditingFinished() signal is fired when text is changed
  * using setText() in playback.
  */
  void triggerTextChangedAndEditingFinished();

private:
  Q_DISABLE_COPY(qtLineEdit)

  bool EditingFinishedPending{ false };
  bool ResetCursorPositionOnEditingFinished{ true };
};
} // namespace extension
} // namespace smtk
#endif
