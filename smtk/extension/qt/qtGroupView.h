//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtGroupView - UI components for Group View
// .SECTION Description
// .SECTION See Also
// qtBaseAttributeView

#ifndef smtk_extension_qtGroupView_h
#define smtk_extension_qtGroupView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"
#include <QIcon>

class qtGroupViewInternals;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtGroupView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtGroupView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtGroupView(const smtk::view::Information& info);
  ~qtGroupView() override;

  qtBaseView* getChildView(int pageIndex);

  virtual void addChildView(qtBaseView*);
  virtual void clearChildViews();
  const QList<qtBaseView*>& childViews() const;

  //Returns true if the view does not contain any information to display - the default
  // behavior is to return false
  bool isEmpty() const override;
  bool isValid() const override;
  const QIcon& alertIcon() const;

public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void updateModelAssociation() override;
  void onShowCategory() override;
  void childModified();

protected:
  void createWidget() override;
  virtual void addGroupBoxEntry(qtBaseView*);
  virtual void addTabEntry(qtBaseView*);
  virtual void addTileEntry(qtBaseView*);

protected Q_SLOTS:
  void updateCurrentTab(int);

private:
  qtGroupViewInternals* m_internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
