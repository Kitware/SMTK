//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtReferenceTreeData_h
#define smtk_extension_qt_qtReferenceTreeData_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/MembershipBadge.h"
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
#include <QPushButton>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>

namespace smtk
{
namespace extension
{

class qtReferenceTreeData;

/**\brief A base class for protected data shared among subclasses of qtReferenceTree.
  *
  * The qtReferenceTreeData class holds widgets and other data common to both
  * the qtComponentTree and qtResourceTree classes.
  */
class SMTKQTEXT_EXPORT qtReferenceTreeData : public QObject
{
  Q_OBJECT

public:
  qtReferenceTreeData();
  ~qtReferenceTreeData() override;

  // SMTK view
  smtk::view::PhraseModelPtr m_phraseModel;

  // Main widget contents
  QGridLayout* m_grid;
  QCheckBox* m_optional{ nullptr }; // Added if the item is optional to reflect IsEnabled().
  QLabel* m_label;                  // The item's label (or name if no label).
  QPushButton* m_copyFromSelection; // A button to copy the selection into the item's entries
  QPushButton* m_copyToSelection;   // A button to copy the item's entries into the selection
  QPushButton* m_clear;             // A button to clear the item's entries
  QToolButton* m_editBtn;           // A button to show a popup used to edit the item's entries
  QTreeView* m_view;                // The Qt widget displaying a tree of items.
  bool m_highlightOnHover{ true };  // Select item members when the cursor hovers in this widget?
  // Which phrases *can* have a membership badge?
  qt::MembershipCriteria m_membershipCriteria{ qt::MembershipCriteria::All };
  // Of the phrases which meet the criteria above, which phrases *do* have a membership badge?
  std::string m_membershipFilter;

  // Selection state of items shown in m_phraseModel, from the MembershipBadge
  // std::map<std::weak_ptr<smtk::resource::PersistentObject>, int,
  //   std::owner_less<std::weak_ptr<smtk::resource::PersistentObject> > >
  //   m_members;

  // Link between Qt model and SMTK phrase-model.
  smtk::extension::qtDescriptivePhraseModel* m_qtModel{ nullptr };
  smtk::extension::qtDescriptivePhraseDelegate* m_qtDelegate{ nullptr };

  smtk::view::PhraseModelObservers::Key m_modelObserverId;

  // Icons used to show item membership
  std::string m_selectedIconURL;
  std::string m_unselectedIconURL;
};
} // namespace extension
} // namespace smtk

#endif
