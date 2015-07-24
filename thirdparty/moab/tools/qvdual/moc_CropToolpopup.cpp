/****************************************************************************
** CropToolPopup meta object code from reading C++ file 'CropToolpopup.h'
**
** Created: Wed Aug 22 09:12:44 2007
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.7   edited Oct 19 16:22 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "CropToolpopup.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *CropToolPopup::className() const
{
    return "CropToolPopup";
}

QMetaObject *CropToolPopup::metaObj = 0;
static QMetaObjectCleanUp cleanUp_CropToolPopup( "CropToolPopup", &CropToolPopup::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString CropToolPopup::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "CropToolPopup", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString CropToolPopup::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "CropToolPopup", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* CropToolPopup::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"radius1_valueChanged", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"Xval1_valueChanged", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"Yval1_valueChanged", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"Zval1_valueChanged", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"Rot1a_valueChanged", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"Rot1b_valueChanged", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"radius2_valueChanged", 1, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"Xval2_valueChanged", 1, param_slot_7 };
    static const QUParameter param_slot_8[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_8 = {"Yval2_valueChanged", 1, param_slot_8 };
    static const QUParameter param_slot_9[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_9 = {"Zval2_valueChanged", 1, param_slot_9 };
    static const QUParameter param_slot_10[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"Rot2a_valueChanged", 1, param_slot_10 };
    static const QUParameter param_slot_11[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_11 = {"Rot2b_valueChanged", 1, param_slot_11 };
    static const QUParameter param_slot_12[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"radius3_valueChanged", 1, param_slot_12 };
    static const QUParameter param_slot_13[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"Xval3_valueChanged", 1, param_slot_13 };
    static const QUParameter param_slot_14[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"Yval3_valueChanged", 1, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"Zval3_valueChanged", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"Rot3a_valueChanged", 1, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"Rot3b_valueChanged", 1, param_slot_17 };
    static const QUParameter param_slot_18[] = {
	{ "togg", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_18 = {"Rev1_toggled", 1, param_slot_18 };
    static const QUParameter param_slot_19[] = {
	{ "togg", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"Rev2_toggled", 1, param_slot_19 };
    static const QUParameter param_slot_20[] = {
	{ "togg", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"Rev3_toggled", 1, param_slot_20 };
    static const QUParameter param_slot_21[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"CropToolType1_highlighted", 1, param_slot_21 };
    static const QUParameter param_slot_22[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"CropToolType2_highlighted", 1, param_slot_22 };
    static const QUParameter param_slot_23[] = {
	{ "num", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_23 = {"CropToolType3_highlighted", 1, param_slot_23 };
    static const QUParameter param_slot_24[] = {
	{ "vtkw", &static_QUType_ptr, "QVTKWidget", QUParameter::In }
    };
    static const QUMethod slot_24 = {"vtk_widget", 1, param_slot_24 };
    static const QUMethod slot_25 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "radius1_valueChanged(int)", &slot_0, QMetaData::Public },
	{ "Xval1_valueChanged(int)", &slot_1, QMetaData::Public },
	{ "Yval1_valueChanged(int)", &slot_2, QMetaData::Public },
	{ "Zval1_valueChanged(int)", &slot_3, QMetaData::Public },
	{ "Rot1a_valueChanged(int)", &slot_4, QMetaData::Public },
	{ "Rot1b_valueChanged(int)", &slot_5, QMetaData::Public },
	{ "radius2_valueChanged(int)", &slot_6, QMetaData::Public },
	{ "Xval2_valueChanged(int)", &slot_7, QMetaData::Public },
	{ "Yval2_valueChanged(int)", &slot_8, QMetaData::Public },
	{ "Zval2_valueChanged(int)", &slot_9, QMetaData::Public },
	{ "Rot2a_valueChanged(int)", &slot_10, QMetaData::Public },
	{ "Rot2b_valueChanged(int)", &slot_11, QMetaData::Public },
	{ "radius3_valueChanged(int)", &slot_12, QMetaData::Public },
	{ "Xval3_valueChanged(int)", &slot_13, QMetaData::Public },
	{ "Yval3_valueChanged(int)", &slot_14, QMetaData::Public },
	{ "Zval3_valueChanged(int)", &slot_15, QMetaData::Public },
	{ "Rot3a_valueChanged(int)", &slot_16, QMetaData::Public },
	{ "Rot3b_valueChanged(int)", &slot_17, QMetaData::Public },
	{ "Rev1_toggled(bool)", &slot_18, QMetaData::Public },
	{ "Rev2_toggled(bool)", &slot_19, QMetaData::Public },
	{ "Rev3_toggled(bool)", &slot_20, QMetaData::Public },
	{ "CropToolType1_highlighted(int)", &slot_21, QMetaData::Public },
	{ "CropToolType2_highlighted(int)", &slot_22, QMetaData::Public },
	{ "CropToolType3_highlighted(int)", &slot_23, QMetaData::Public },
	{ "vtk_widget(QVTKWidget*)", &slot_24, QMetaData::Public },
	{ "languageChange()", &slot_25, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"CropToolPopup", parentObject,
	slot_tbl, 26,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_CropToolPopup.setMetaObject( metaObj );
    return metaObj;
}

void* CropToolPopup::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "CropToolPopup" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool CropToolPopup::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: radius1_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 1: Xval1_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 2: Yval1_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 3: Zval1_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 4: Rot1a_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 5: Rot1b_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 6: radius2_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 7: Xval2_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 8: Yval2_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 9: Zval2_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 10: Rot2a_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 11: Rot2b_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 12: radius3_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 13: Xval3_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 14: Yval3_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 15: Zval3_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 16: Rot3a_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 17: Rot3b_valueChanged((int)static_QUType_int.get(_o+1)); break;
    case 18: Rev1_toggled((bool)static_QUType_bool.get(_o+1)); break;
    case 19: Rev2_toggled((bool)static_QUType_bool.get(_o+1)); break;
    case 20: Rev3_toggled((bool)static_QUType_bool.get(_o+1)); break;
    case 21: CropToolType1_highlighted((int)static_QUType_int.get(_o+1)); break;
    case 22: CropToolType2_highlighted((int)static_QUType_int.get(_o+1)); break;
    case 23: CropToolType3_highlighted((int)static_QUType_int.get(_o+1)); break;
    case 24: vtk_widget((QVTKWidget*)static_QUType_ptr.get(_o+1)); break;
    case 25: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool CropToolPopup::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool CropToolPopup::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool CropToolPopup::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
