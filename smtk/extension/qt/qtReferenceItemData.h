//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtReferenceItemData_h
#define smtk_extension_qt_qtReferenceItemData_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/PhraseModel.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>

namespace smtk
{
namespace extension
{

class qtReferenceItemData;

/**\brief A base class for protected data shared between component and resource items.
  *
  * The qtReferenceItemData class holds widgets and other data common to both
  * the qtComponentItem and qtResourceItem classes.
  */
class SMTKQTEXT_EXPORT qtReferenceItemData : public QObject
{
  Q_OBJECT

public:
  qtReferenceItemData();
  virtual ~qtReferenceItemData();

  // SMTK view
  smtk::view::PhraseModelPtr m_phraseModel;

  // Main widget contents
  QGridLayout* m_grid;
  QCheckBox* m_optional;  // Added if the item is optional to reflect IsEnabled().
  QLabel* m_label;        // The item's label (or name if no label).
  QLabel* m_synopsis;     // A live summary of the item's entries and acceptability
  QPushButton* m_editBtn; // A button to show a popup used to edit the item's entries
  QPushButton*
    m_exportSeln; // A button to one-shot export the item's entries to an application selection
  QPushButton* m_importSeln; // A button to one-shot import an application selection to the entries
  QPushButton*
    m_linkSeln; // A button to link an application selection to the item's entries on an ongoing basis.

  // Popup widget contents
  QDialog* m_popup;
  QLabel* m_popupSynopsis;
  QPushButton* m_popupDone;
  QListView* m_popupList;

  // Link between Qt and SMTK
  smtk::extension::qtDescriptivePhraseModel* m_qtModel;
  smtk::extension::qtDescriptivePhraseDelegate* m_qtDelegate;
};
}
}

#endif
