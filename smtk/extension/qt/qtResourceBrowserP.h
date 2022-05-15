//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtResourceBrowserP_h
#define smtk_extension_qt_qtResourceBrowserP_h

#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/view/SelectionObserver.h"

#include <QPointer>

#include <string>

class QAbstractItemModel;
class QAbstractItemView;
class QTreeView;
class QVBoxLayout;

namespace smtk
{
namespace extension
{

class qtDescriptivePhraseDelegate;

/// A "private" object for use only by qtResourceBrowser and its subclasses.
class SMTKQTEXT_EXPORT qtResourceBrowser::Internal // : public Ui::qtResourceBrowser
{
public:
  Internal();
  ~Internal();

  void setup(
    qtResourceBrowser* self,
    const smtk::view::PhraseModelPtr& phraseModel,
    const std::string& viewName,
    QAbstractItemModel* qmodel,
    QWidget* parent,
    const std::shared_ptr<smtk::view::Selection>& selection =
      std::shared_ptr<smtk::view::Selection>());

  smtk::extension::qtDescriptivePhraseModel* descriptivePhraseModel() const;
  void setDescriptivePhraseModel(QAbstractItemModel* qmodel);

  QWidget* m_container{ nullptr };
  QVBoxLayout* m_layout{ nullptr };
  QTreeView* m_view{ nullptr };
  QPointer<qtResourceBrowser> m_self;
  QPointer<QAbstractItemModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::PhraseModelPtr m_phraseModel;
  smtk::view::SelectionPtr m_seln; // TODO: This assumes there is only 1 server connection
  smtk::view::SelectionObservers::Key m_selnHandle; // TODO: Same assumption as m_seln
  int m_selnValue;
  int m_hoverValue;
  std::string m_selnSource; // TODO: This assumes there is only 1 panel (or that all should share)
  std::string m_selnLabel;
  std::string m_hoverLabel;
  std::string m_viewName;
  std::string m_resourceTreeType; // "default" or specific type.
  int m_resourceTreeStyle{ -1 };  // Which default subphrase generator should be used?

  // Set to true when inside sendSMTKSelectionToPanel.
  // Used to avoid updating the SMTK selection from the panel while
  // the panel is being updated from SMTK:
  bool m_updatingPanelSelectionFromSMTK{ false };
};
} // namespace extension
} // namespace smtk
#endif // smtk_extension_qt_qtResourceBrowserP_h
