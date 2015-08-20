/****************************************************************************
** Form implementation generated from reading ui file 'SheetDiagramPopup.ui'
**
** Created: Wed Aug 22 09:12:45 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "SheetDiagramPopup.h"

#include <qvariant.h>
#include <QVTKWidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "QVTKWidget.h"
#include "SheetDiagramPopup.ui.h"
/*
 *  Constructs a SheetDiagramPopup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SheetDiagramPopup::SheetDiagramPopup( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "SheetDiagramPopup" );
    setMouseTracking( FALSE );
    setFocusPolicy( QDialog::NoFocus );

    sheetDiagram = new QVTKWidget( this, "sheetDiagram" );
    sheetDiagram->setGeometry( QRect( 0, 0, 250, 250 ) );
    sheetDiagram->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sheetDiagram->sizePolicy().hasHeightForWidth() ) );
    sheetDiagram->setBackgroundOrigin( QVTKWidget::WindowOrigin );
    languageChange();
    resize( QSize(494, 484).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
SheetDiagramPopup::~SheetDiagramPopup()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SheetDiagramPopup::languageChange()
{
    setCaption( tr( "Sheet Diagram" ) );
}

