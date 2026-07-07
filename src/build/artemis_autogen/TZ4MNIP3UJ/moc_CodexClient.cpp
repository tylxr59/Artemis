/****************************************************************************
** Meta object code from reading C++ file 'CodexClient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../artemis-git/src/providers/codex/CodexClient.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CodexClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7Artemis11CodexClientE_t {};
} // unnamed namespace

template <> constexpr inline auto Artemis::CodexClient::qt_create_metaobjectdata<qt_meta_tag_ZN7Artemis11CodexClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Artemis::CodexClient",
        "readyChanged",
        "versionChanged",
        "ready",
        "version"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'ready'
        QtMocHelpers::PropertyData<bool>(3, QMetaType::Bool, QMC::DefaultPropertyFlags, 0x70000000 | 1),
        // property 'version'
        QtMocHelpers::PropertyData<QString>(4, QMetaType::QString, QMC::DefaultPropertyFlags, 0x70000000 | 2),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CodexClient, qt_meta_tag_ZN7Artemis11CodexClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Artemis::CodexClient::staticMetaObject = { {
    QMetaObject::SuperData::link<AgentProvider::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis11CodexClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis11CodexClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Artemis11CodexClientE_t>.metaTypes,
    nullptr
} };

void Artemis::CodexClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CodexClient *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->ready(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->version(); break;
        default: break;
        }
    }
}

const QMetaObject *Artemis::CodexClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Artemis::CodexClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis11CodexClientE_t>.strings))
        return static_cast<void*>(this);
    return AgentProvider::qt_metacast(_clname);
}

int Artemis::CodexClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AgentProvider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
namespace CheckNotifySignalValidity_ZN7Artemis11CodexClientE {
template<typename T> using has_nullary_readyChanged = decltype(std::declval<T>().readyChanged());
template<typename T> using has_unary_readyChanged = decltype(std::declval<T>().readyChanged(std::declval<bool>()));
static_assert(qxp::is_detected_v<has_nullary_readyChanged, Artemis::CodexClient> || qxp::is_detected_v<has_unary_readyChanged, Artemis::CodexClient>,
              "NOTIFY signal readyChanged does not exist in class (or is private in its parent)");
template<typename T> using has_nullary_versionChanged = decltype(std::declval<T>().versionChanged());
template<typename T> using has_unary_versionChanged = decltype(std::declval<T>().versionChanged(std::declval<QString>()));
static_assert(qxp::is_detected_v<has_nullary_versionChanged, Artemis::CodexClient> || qxp::is_detected_v<has_unary_versionChanged, Artemis::CodexClient>,
              "NOTIFY signal versionChanged does not exist in class (or is private in its parent)");
}
QT_WARNING_POP
