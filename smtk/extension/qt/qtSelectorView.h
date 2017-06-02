//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_qtSelectorView_h
#define __smtk_extension_qtSelectorView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseView.h"

/**\brief Provides the QT UI for a Selector View.
  *
  * A Selector View is similar to a Group View in that it is a set of Views.
  * However, unlike the Group View only one of its children views is visible.
  * The View that is visible is based on the value of a Discrete Item contained
  * in an attribute.  Currently that item should be the only item in the attribute.
  *
  * The structure of the Selector View has the following Top-Level view attributes in additon to
  * those that all views have:
  *
  * - Type = "Selector"
  * - SelectorName - The name of the attribute to be used for selection. if the attribute
  * does not exist it will be created
  * - SelectorType - The name of the selector attribute's definition.
  *
  * In terms of children components, the Selector View has a similar structure  as Group Views.
  * The Views component represent the list of children views used by the Selector.
  * Each child is represented by a View component that includes the following view attributes:
  *
  * - Title - name of the child view
  * - Enum - the enumeration value that the selector attribute's item needs to be set to for
  * this view to be visible
  *
  * \sa qtGroupView
  */

class qtSelectorViewInternals;

namespace smtk
{
namespace extension
{
class SMTKQTEXT_EXPORT qtSelectorView : public qtBaseView
{
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const ViewInfo& info);
  qtSelectorView(const ViewInfo& info);
  virtual ~qtSelectorView();

  void getChildView(const std::string& viewType, QList<qtBaseView*>& views);
  qtBaseView* getChildView(int pageIndex);

  virtual void addChildView(qtBaseView*, int viewEnumIndex);
  virtual void clearChildViews();
  const QList<qtBaseView*>& childViews() const;

public slots:
  virtual void updateUI();
  virtual void showAdvanceLevelOverlay(bool show);
  virtual void updateModelAssociation();
  virtual void onShowCategory();
  void selectionChanged();

protected:
  virtual void createWidget();
  bool createSelector();
  bool createChildren();

private:
  qtSelectorViewInternals* Internals;

}; // class
}; // namespace attribute
}; // namespace smtk

#endif
