//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtDescriptivePhraseEditor_h
#define __smtk_extension_qtDescriptivePhraseEditor_h

#include "smtk/extension/qt/Exports.h"
#include <QWidget>

class QLineEdit;

namespace smtk
{
namespace extension
{

/**\brief Allow user edits to an smtk::model::Resource instance via qtDescriptivePhraseModel.
  *
  */
class SMTKQTEXT_EXPORT qtDescriptivePhraseEditor : public QWidget
{
  Q_OBJECT
public:
  qtDescriptivePhraseEditor(QWidget* parent = 0);
  ~qtDescriptivePhraseEditor() override;

  //QSize sizeHint() const;
  QString title() const;
  void setTitle(const QString& text);

signals:
  void editingFinished();

protected:
  QLineEdit* m_title;
};

} // namespace model
} // namespace smtk

#endif // __smtk_extension_qtDescriptivePhraseEditor_h
