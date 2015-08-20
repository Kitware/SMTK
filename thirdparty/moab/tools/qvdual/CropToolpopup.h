/****************************************************************************
** Form interface generated from reading ui file 'CropToolpopup.ui'
**
** Created: Tue Aug 7 10:19:10 2007
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.7   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CROPTOOLPOPUP_H
#define CROPTOOLPOPUP_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QComboBox;
class QSpinBox;
class QCheckBox;
class CropTool;
class QVTKWidget;

class CropToolPopup : public QDialog
{
    Q_OBJECT

public:
    CropToolPopup( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~CropToolPopup();

    QLabel* textLabel1_2;
    QLabel* textLabel1;
    QLabel* textLabel1_3_2_2_2;
    QLabel* textLabel1_3;
    QComboBox* CropToolType2;
    QComboBox* CropToolType3;
    QSpinBox* Yval1;
    QSpinBox* Zval1;
    QSpinBox* Xval2;
    QSpinBox* Yval2;
    QSpinBox* Zval2;
    QSpinBox* Xval3;
    QSpinBox* Yval3;
    QSpinBox* Zval3;
    QLabel* textLabel1_3_2;
    QLabel* textLabel1_3_2_3;
    QCheckBox* Rev1;
    QCheckBox* Rev2;
    QCheckBox* Rev3;
    QSpinBox* radius3;
    QSpinBox* radius2;
    QSpinBox* radius1;
    QLabel* textLabel1_3_2_2;
    QComboBox* CropToolType1;
    QSpinBox* Xval1;
    QSpinBox* Rot1a;
    QSpinBox* Rot1b;
    QSpinBox* Rot2a;
    QSpinBox* Rot2b;
    QSpinBox* Rot3a;
    QSpinBox* Rot3b;

    virtual void init();
    virtual void destroy();

public slots:
    virtual void radius1_valueChanged( int num );
    virtual void Xval1_valueChanged( int num );
    virtual void Yval1_valueChanged( int num );
    virtual void Zval1_valueChanged( int num );
    virtual void Rot1a_valueChanged( int num );
    virtual void Rot1b_valueChanged( int num );
    virtual void radius2_valueChanged( int num );
    virtual void Xval2_valueChanged( int num );
    virtual void Yval2_valueChanged( int num );
    virtual void Zval2_valueChanged( int num );
    virtual void Rot2a_valueChanged( int num );
    virtual void Rot2b_valueChanged( int num );
    virtual void radius3_valueChanged( int num );
    virtual void Xval3_valueChanged( int num );
    virtual void Yval3_valueChanged( int num );
    virtual void Zval3_valueChanged( int num );
    virtual void Rot3a_valueChanged( int num );
    virtual void Rot3b_valueChanged( int num );
    virtual void Rev1_toggled( bool togg );
    virtual void Rev2_toggled( bool togg );
    virtual void Rev3_toggled( bool togg );
    virtual void CropToolType1_highlighted( int num );
    virtual void CropToolType2_highlighted( int num );
    virtual void CropToolType3_highlighted( int num );
    virtual void vtk_widget( QVTKWidget * vtkw );

protected:
    QVTKWidget *vtkWidget;


protected slots:
    virtual void languageChange();

private:
    CropTool *cropTool;

};

#endif // CROPTOOLPOPUP_H
