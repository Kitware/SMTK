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
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

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
  QCheckBox* m_optional;            // Added if the item is optional to reflect IsEnabled().
  QLabel* m_label;                  // The item's label (or name if no label).
  QLabel* m_synopsis;               // A live summary of the item's entries and acceptability
  QPushButton* m_copyFromSelection; // A button to copy the selection into the item's entries
  QPushButton* m_copyToSelection;   // A button to copy the item's entries into the selection
  QPushButton* m_clear;             // A button to clear the item's entries
  QToolButton* m_editBtn;           // A button to show a popup used to edit the item's entries
  QPushButton* m_linkSeln; // A button to link an app. selection to the item's entries permanently.

  // Popup widget contents
  QDialog* m_popup;
  QVBoxLayout* m_popupLayout;
  QListView* m_popupList;
  bool m_alreadyClosingPopup; // Set when synchronizeAndHide() should **not** hide the QMenu.

  // Selection state of items shown in m_phraseModel:
  std::map<std::weak_ptr<smtk::resource::PersistentObject>, int,
    std::owner_less<std::weak_ptr<smtk::resource::PersistentObject> > >
    m_members;

  // Link between Qt and SMTK
  smtk::extension::qtDescriptivePhraseModel* m_qtModel;
  smtk::extension::qtDescriptivePhraseDelegate* m_qtDelegate;

  smtk::view::PhraseModelObservers::Key m_modelObserverId;

  // Icons used to show item membership
  std::string m_selectedIconURL;
  std::string m_unselectedIconURL;
};
}
}

#endif
