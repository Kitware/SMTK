//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtInstancedView - UI components for attribute Instanced View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_attribute_qtInstancedView_h
#define __smtk_attribute_qtInstancedView_h

#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/Exports.h"

class qtInstancedViewInternals;
class QScrollArea;

namespace smtk
{
  namespace attribute
  {
    class SMTKQTEXT_EXPORT qtInstancedView : public qtBaseView
    {
      Q_OBJECT

    public:
      static qtBaseView *createViewWidget(const ViewInfo &info);

      qtInstancedView(const ViewInfo &info);
      virtual ~qtInstancedView();

    public slots:
      virtual void showAdvanceLevelOverlay(bool show);
      virtual void requestModelEntityAssociation();
      virtual void onShowCategory()
       { this->updateAttributeData(); }

    protected:
      virtual void updateAttributeData();
      virtual void createWidget( );

    private:

      qtInstancedViewInternals *Internals;

    }; // class
  }; // namespace attribute
}; // namespace smtk


#endif
