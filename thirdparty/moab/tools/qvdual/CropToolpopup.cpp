/****************************************************************************
** Form implementation generated from reading ui file 'CropToolpopup.ui'
**
** Created: Wed Aug 22 09:12:41 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "CropToolpopup.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "CropTool.hpp"
#include "QVTKWidget.h"
#include "vtkRenderWindow.h"
#include "CropToolpopup.ui.h"
/*
 *  Constructs a CropToolPopup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CropToolPopup::CropToolPopup( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CropToolPopup" );
    setFocusPolicy( QDialog::StrongFocus );

    textLabel1_2 = new QLabel( this, "textLabel1_2" );
    textLabel1_2->setGeometry( QRect( 260, 10, 20, 21 ) );

    textLabel1 = new QLabel( this, "textLabel1" );
    textLabel1->setGeometry( QRect( 190, 10, 20, 21 ) );

    textLabel1_3_2_2_2 = new QLabel( this, "textLabel1_3_2_2_2" );
    textLabel1_3_2_2_2->setGeometry( QRect( 10, 10, 37, 21 ) );

    textLabel1_3 = new QLabel( this, "textLabel1_3" );
    textLabel1_3->setGeometry( QRect( 330, 10, 20, 21 ) );

    CropToolType2 = new QComboBox( FALSE, this, "CropToolType2" );
    CropToolType2->setGeometry( QRect( 0, 100, 90, 21 ) );

    CropToolType3 = new QComboBox( FALSE, this, "CropToolType3" );
    CropToolType3->setGeometry( QRect( 0, 160, 90, 21 ) );

    Yval1 = new QSpinBox( this, "Yval1" );
    Yval1->setGeometry( QRect( 240, 40, 59, 21 ) );
    Yval1->setMaxValue( 10000 );
    Yval1->setMinValue( -10000 );

    Zval1 = new QSpinBox( this, "Zval1" );
    Zval1->setGeometry( QRect( 310, 40, 59, 21 ) );
    Zval1->setMaxValue( 10000 );
    Zval1->setMinValue( -10000 );

    Xval2 = new QSpinBox( this, "Xval2" );
    Xval2->setGeometry( QRect( 170, 100, 59, 21 ) );
    Xval2->setMaxValue( 10000 );
    Xval2->setMinValue( -10000 );

    Yval2 = new QSpinBox( this, "Yval2" );
    Yval2->setGeometry( QRect( 240, 100, 59, 21 ) );
    Yval2->setMaxValue( 10000 );
    Yval2->setMinValue( -10000 );

    Zval2 = new QSpinBox( this, "Zval2" );
    Zval2->setGeometry( QRect( 310, 100, 59, 21 ) );
    Zval2->setMaxValue( 10000 );
    Zval2->setMinValue( -10000 );

    Xval3 = new QSpinBox( this, "Xval3" );
    Xval3->setGeometry( QRect( 170, 160, 59, 21 ) );
    Xval3->setMaxValue( 10000 );
    Xval3->setMinValue( -10000 );

    Yval3 = new QSpinBox( this, "Yval3" );
    Yval3->setGeometry( QRect( 240, 160, 59, 21 ) );
    Yval3->setMaxValue( 10000 );
    Yval3->setMinValue( -10000 );

    Zval3 = new QSpinBox( this, "Zval3" );
    Zval3->setGeometry( QRect( 310, 160, 59, 21 ) );
    Zval3->setMaxValue( 10000 );
    Zval3->setMinValue( -10000 );

    textLabel1_3_2 = new QLabel( this, "textLabel1_3_2" );
    textLabel1_3_2->setGeometry( QRect( 400, 10, 37, 21 ) );

    textLabel1_3_2_3 = new QLabel( this, "textLabel1_3_2_3" );
    textLabel1_3_2_3->setGeometry( QRect( 460, 10, 37, 21 ) );

    Rev1 = new QCheckBox( this, "Rev1" );
    Rev1->setGeometry( QRect( 470, 40, 20, 20 ) );

    Rev2 = new QCheckBox( this, "Rev2" );
    Rev2->setGeometry( QRect( 470, 100, 20, 20 ) );

    Rev3 = new QCheckBox( this, "Rev3" );
    Rev3->setGeometry( QRect( 470, 160, 20, 20 ) );

    radius3 = new QSpinBox( this, "radius3" );
    radius3->setGeometry( QRect( 100, 160, 59, 21 ) );
    radius3->setMaxValue( 10000 );
    radius3->setMinValue( 0 );
    radius3->setValue( 1 );

    radius2 = new QSpinBox( this, "radius2" );
    radius2->setGeometry( QRect( 100, 100, 59, 21 ) );
    radius2->setMaxValue( 10000 );
    radius2->setMinValue( 0 );
    radius2->setValue( 1 );

    radius1 = new QSpinBox( this, "radius1" );
    radius1->setGeometry( QRect( 100, 40, 59, 21 ) );
    radius1->setMaxValue( 10000 );
    radius1->setMinValue( 0 );
    radius1->setValue( 1 );

    textLabel1_3_2_2 = new QLabel( this, "textLabel1_3_2_2" );
    textLabel1_3_2_2->setGeometry( QRect( 110, 10, 30, 21 ) );

    CropToolType1 = new QComboBox( FALSE, this, "CropToolType1" );
    CropToolType1->setGeometry( QRect( 0, 40, 90, 21 ) );

    Xval1 = new QSpinBox( this, "Xval1" );
    Xval1->setGeometry( QRect( 170, 40, 59, 21 ) );
    Xval1->setMaxValue( 10000 );
    Xval1->setMinValue( -10000 );

    Rot1a = new QSpinBox( this, "Rot1a" );
    Rot1a->setGeometry( QRect( 390, 40, 59, 21 ) );
    Rot1a->setMaxValue( 180 );
    Rot1a->setMinValue( -180 );
    Rot1a->setLineStep( 5 );

    Rot1b = new QSpinBox( this, "Rot1b" );
    Rot1b->setGeometry( QRect( 390, 60, 59, 21 ) );
    Rot1b->setMaxValue( 180 );
    Rot1b->setMinValue( -180 );
    Rot1b->setLineStep( 5 );

    Rot2a = new QSpinBox( this, "Rot2a" );
    Rot2a->setGeometry( QRect( 390, 100, 59, 21 ) );
    Rot2a->setMaxValue( 180 );
    Rot2a->setMinValue( -180 );
    Rot2a->setLineStep( 5 );

    Rot2b = new QSpinBox( this, "Rot2b" );
    Rot2b->setGeometry( QRect( 390, 120, 59, 21 ) );
    Rot2b->setMaxValue( 180 );
    Rot2b->setMinValue( -180 );
    Rot2b->setLineStep( 5 );

    Rot3a = new QSpinBox( this, "Rot3a" );
    Rot3a->setGeometry( QRect( 390, 160, 59, 21 ) );
    Rot3a->setMaxValue( 180 );
    Rot3a->setMinValue( -180 );
    Rot3a->setLineStep( 5 );

    Rot3b = new QSpinBox( this, "Rot3b" );
    Rot3b->setGeometry( QRect( 390, 180, 59, 21 ) );
    Rot3b->setMaxValue( 180 );
    Rot3b->setMinValue( -180 );
    Rot3b->setLineStep( 5 );
    languageChange();
    resize( QSize(511, 220).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( radius1, SIGNAL( valueChanged(int) ), this, SLOT( radius1_valueChanged(int) ) );
    connect( Xval1, SIGNAL( valueChanged(int) ), this, SLOT( Xval1_valueChanged(int) ) );
    connect( Yval1, SIGNAL( valueChanged(int) ), this, SLOT( Yval1_valueChanged(int) ) );
    connect( Zval1, SIGNAL( valueChanged(int) ), this, SLOT( Zval1_valueChanged(int) ) );
    connect( Rot1a, SIGNAL( valueChanged(int) ), this, SLOT( Rot1a_valueChanged(int) ) );
    connect( Rot1b, SIGNAL( valueChanged(int) ), this, SLOT( Rot1b_valueChanged(int) ) );
    connect( radius2, SIGNAL( valueChanged(int) ), this, SLOT( radius2_valueChanged(int) ) );
    connect( Xval2, SIGNAL( valueChanged(int) ), this, SLOT( Xval2_valueChanged(int) ) );
    connect( Yval2, SIGNAL( valueChanged(int) ), this, SLOT( Yval2_valueChanged(int) ) );
    connect( Zval2, SIGNAL( valueChanged(int) ), this, SLOT( Zval2_valueChanged(int) ) );
    connect( Rot2a, SIGNAL( valueChanged(int) ), this, SLOT( Rot2a_valueChanged(int) ) );
    connect( Rot2b, SIGNAL( valueChanged(int) ), this, SLOT( Rot2b_valueChanged(int) ) );
    connect( radius3, SIGNAL( valueChanged(int) ), this, SLOT( radius3_valueChanged(int) ) );
    connect( Xval3, SIGNAL( valueChanged(int) ), this, SLOT( Xval3_valueChanged(int) ) );
    connect( Yval3, SIGNAL( valueChanged(int) ), this, SLOT( Yval3_valueChanged(int) ) );
    connect( Zval3, SIGNAL( valueChanged(int) ), this, SLOT( Zval3_valueChanged(int) ) );
    connect( Rot3a, SIGNAL( valueChanged(int) ), this, SLOT( Rot3a_valueChanged(int) ) );
    connect( Rot3b, SIGNAL( valueChanged(int) ), this, SLOT( Rot3b_valueChanged(int) ) );
    connect( Rev1, SIGNAL( toggled(bool) ), this, SLOT( Rev1_toggled(bool) ) );
    connect( Rev2, SIGNAL( toggled(bool) ), this, SLOT( Rev2_toggled(bool) ) );
    connect( Rev3, SIGNAL( toggled(bool) ), this, SLOT( Rev3_toggled(bool) ) );
    connect( CropToolType1, SIGNAL( highlighted(int) ), this, SLOT( CropToolType1_highlighted(int) ) );
    connect( CropToolType2, SIGNAL( highlighted(int) ), this, SLOT( CropToolType2_highlighted(int) ) );
    connect( CropToolType3, SIGNAL( highlighted(int) ), this, SLOT( CropToolType3_highlighted(int) ) );
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CropToolPopup::~CropToolPopup()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CropToolPopup::languageChange()
{
    setCaption( tr( "Crop" ) );
    textLabel1_2->setText( tr( "Y" ) );
    textLabel1->setText( tr( "X" ) );
    textLabel1_3_2_2_2->setText( tr( "Type" ) );
    textLabel1_3->setText( tr( "Z" ) );
    CropToolType2->clear();
    CropToolType2->insertItem( tr( "None" ) );
    CropToolType2->insertItem( tr( "Plane" ) );
    CropToolType2->insertItem( tr( "Cylinder" ) );
    CropToolType2->insertItem( tr( "Sphere" ) );
    CropToolType3->clear();
    CropToolType3->insertItem( tr( "None" ) );
    CropToolType3->insertItem( tr( "Plane" ) );
    CropToolType3->insertItem( tr( "Cylinder" ) );
    CropToolType3->insertItem( tr( "Sphere" ) );
    textLabel1_3_2->setText( tr( "Rot" ) );
    textLabel1_3_2_3->setText( tr( "Rev" ) );
    Rev1->setText( tr( "checkBox1" ) );
    Rev2->setText( tr( "checkBox1" ) );
    Rev3->setText( tr( "checkBox1" ) );
    textLabel1_3_2_2->setText( tr( "Rad" ) );
    CropToolType1->clear();
    CropToolType1->insertItem( tr( "None" ) );
    CropToolType1->insertItem( tr( "PlaneX" ) );
    CropToolType1->insertItem( tr( "PlaneY" ) );
    CropToolType1->insertItem( tr( "PlaneZ" ) );
    CropToolType1->insertItem( tr( "Cylinder" ) );
    CropToolType1->insertItem( tr( "Sphere" ) );
}

