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

  void setup(qtResourceBrowser* self, const smtk::view::PhraseModelPtr& phraseModel,
    const std::string& viewName, QAbstractItemModel* qmodel, QWidget* parent);

  smtk::extension::qtDescriptivePhraseModel* descriptivePhraseModel() const;

  QVBoxLayout* m_layout;
  QTreeView* m_view;
  QPointer<qtResourceBrowser> m_self;
  QPointer<QAbstractItemModel> m_model;
  QPointer<smtk::extension::qtDescriptivePhraseDelegate> m_delegate;
  std::map<smtk::resource::ManagerPtr, int> m_observers;
  smtk::view::PhraseModelPtr m_phraseModel;
  smtk::view::SelectionPtr m_seln; // TODO: This assumes there is only 1 server connection
  int m_selnHandle;                // TODO: Same assumption as m_seln
  int m_selnValue;
  int m_hoverValue;
  std::string m_selnSource; // TODO: This assumes there is only 1 panel (or that all should share)
  std::string m_selnLabel;
  std::string m_hoverLabel;
  std::map<smtk::common::UUID, int> m_visibleThings;
  int m_resourceTreeStyle; // Which subphrase generator should be used?

  // Set to true when inside sendSMTKSelectionToPanel.
  // Used to avoid updating the SMTK selection from the panel while
  // the panel is being updated from SMTK:
  bool m_updatingPanelSelectionFromSMTK;
};
}
}
#endif // smtk_extension_qt_qtResourceBrowserP_h
