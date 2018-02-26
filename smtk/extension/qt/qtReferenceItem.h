//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtReferenceItem_h
#define smtk_extension_qt_qtReferenceItem_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtItem.h"

namespace smtk
{
namespace extension
{

class qtReferenceItemData;

/**\brief A base class for component and resource items.
  *
  * The qtReferenceItem class provides a uniform GUI for selecting entries that
  * populate an attribute's item while abstracting away the types of those
  * entries.
  */
class SMTKQTEXT_EXPORT qtReferenceItem : public qtItem
{
  Q_OBJECT
  using Superclass = qtItem;

public:
  qtReferenceItem(smtk::attribute::ItemPtr, QWidget* parent, qtBaseView* bview);
  virtual ~qtReferenceItem();

protected slots:
  virtual void selectionLinkToggled(bool linked);
  virtual void setOutputOptional(int state);

protected:
  virtual void createWidget() override;

  virtual void clearWidgets();
  virtual void updateUI();

  qtReferenceItemData* m_p;
};
}
}

#endif
