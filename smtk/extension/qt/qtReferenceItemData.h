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

#include "smtk/view/Configuration.h"
#include "smtk/view/PhraseModel.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPointer>
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
  ~qtReferenceItemData() override;

  // SMTK view
  smtk::view::PhraseModelPtr m_phraseModel;

  /// Main widget contents
  QPointer<QGridLayout> m_grid;
  /// Added if the item is optional to reflect IsEnabled().
  QPointer<QCheckBox> m_optional{ nullptr };
  /// The item's label (or name if no label).
  QPointer<QLabel> m_label;
  /// A live summary of the item's entries and acceptability
  QPointer<QLabel> m_synopsis;
  /// A button to copy the selection into the item's entries
  QPointer<QPushButton> m_copyFromSelection;
  /// A button to copy the item's entries into the selection
  QPointer<QPushButton> m_copyToSelection;
  /// A button to clear the item's entries
  QPointer<QPushButton> m_clear;
  /// A button to show a popup used to edit the item's entries
  QPointer<QToolButton> m_editBtn;
  /// A button to link an app. selection to the item's entries permanently.
  QPointer<QPushButton> m_linkSeln;

  /// Popup widget contents
  QPointer<QDialog> m_popup;
  QPointer<QVBoxLayout> m_popupLayout;
  QPointer<QListView> m_popupList;
  /// Set when synchronizeAndHide() should **not** hide the QMenu.
  bool m_alreadyClosingPopup{ false };

  // Selection state of items shown in m_phraseModel, from the MembershipBadge
  // std::map<std::weak_ptr<smtk::resource::PersistentObject>, int,
  //   std::owner_less<std::weak_ptr<smtk::resource::PersistentObject> > >
  //   m_members;

  /// Link between Qt and SMTK
  smtk::extension::qtDescriptivePhraseModel* m_qtModel{ nullptr };
  smtk::extension::qtDescriptivePhraseDelegate* m_qtDelegate{ nullptr };

  smtk::view::PhraseModelObservers::Key m_modelObserverId;

  /// Icons used to show item membership
  std::string m_selectedIconURL;
  std::string m_unselectedIconURL;
};
} // namespace extension
} // namespace smtk

#endif
