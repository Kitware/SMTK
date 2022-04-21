//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtAnalysisView_h
#define smtk_extension_qtAnalysisView_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <set>

namespace smtk
{
namespace extension
{

/**\brief Provides the QT UI for an Analysis View.
  *
  * An Analysis View is a specialized view for choosing the types of analyses the user wants to
  * perform.  These choices are persistent and can be used by an export operation instead of having
  * the operator ask the user what types of analyses should be performed.
  *
  * Unlike other views the Analysis View will construct both an Attribute Definition and
  * corresponding Attribute when needed.  The Attribute Definition is based on the Analysis Information
  * stored in the Attribute Resource.  Any Analysis that is referred to by another will be represented
  * as a Group Item.  All other Analyses will be represented as a Void Item.
  *
  * The View also controls which categories are permitted to be displayed and/or selected.  The set
  * is union of all of the selected Analyses' categories.
  *
  * The following is an example of a Analysis View:
  * ```
  * <View Type="Analysis" Title="Analysis" AnalysisAttributeName="truchasAnalysis" AnalysisAttributeType="truchasAnalysisDefinition">
  * </View>
  * ```
  * AnalysisAttributeType is the name of the Attribute Definition the view will create to represent the
  * Analysis Structure (if needed)
  *
  * AnalysisAttributeName is the name of the Attribute the view will create to represent the
  * Analysis  (if needed)
  *
  * \sa qtBaseAttributeView
  */
class SMTKQTEXT_EXPORT qtAnalysisView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtAnalysisView);

  /// \brief Factory method to create a qtAnalysisView from a smtk::view::Information
  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  qtAnalysisView(const smtk::view::Information& info);
  ~qtAnalysisView() override;

  bool isValid() const override;

public Q_SLOTS:
  /// \brief Slot used to update the UI when Analysis Attribute changes
  void analysisAttributeChanged();

protected:
  void createWidget() override;
  /// \brief Method to assembly the categories represented by an item in the Analysis Attribute
  void processAnalysisItem(smtk::attribute::ConstItemPtr item, std::set<std::string>& cats);
  /// \brief Override's qtBaseView Test
  ///
  /// Since the items of the Analysis Attribute have no categories, this view turns off this
  // check by always returning true.
  bool categoryTest(const smtk::attribute::ItemPtr&) const override;

  /// \brief Method to update the category filtering based on the state of the Analysis Attribute
  void analysisChanged(bool attributeChanged);

private:
  smtk::attribute::AttributePtr m_analysisAttribute; ///< Analysis Attribute used by the View
  smtk::extension::qtAttribute* m_qtAnalysisAttribute;
};
} // namespace extension
} // namespace smtk

#endif
