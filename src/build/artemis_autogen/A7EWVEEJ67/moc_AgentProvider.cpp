/****************************************************************************
** Meta object code from reading C++ file 'AgentProvider.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../artemis-git/src/domain/AgentProvider.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AgentProvider.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7Artemis13AgentProviderE_t {};
} // unnamed namespace

template <> constexpr inline auto Artemis::AgentProvider::qt_create_metaobjectdata<qt_meta_tag_ZN7Artemis13AgentProviderE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Artemis::AgentProvider",
        "readyChanged",
        "",
        "ready",
        "setupChanged",
        "versionChanged",
        "activeTurnStarted",
        "threadId",
        "turnId",
        "tokenUsageUpdated",
        "contextTokens",
        "totalProcessedTokens",
        "modelContextWindow",
        "userInputRequested",
        "itemId",
        "QVariantList",
        "questions",
        "domainEvent",
        "type",
        "title",
        "content",
        "QVariantMap",
        "metadata",
        "providerError",
        "message"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'readyChanged'
        QtMocHelpers::SignalData<void(bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 },
        }}),
        // Signal 'setupChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'versionChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeTurnStarted'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::QString, 8 },
        }}),
        // Signal 'tokenUsageUpdated'
        QtMocHelpers::SignalData<void(const QString &, qint64, qint64, qint64)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::LongLong, 10 }, { QMetaType::LongLong, 11 }, { QMetaType::LongLong, 12 },
        }}),
        // Signal 'userInputRequested'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &, const QVariantList &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::QString, 8 }, { QMetaType::QString, 14 }, { 0x80000000 | 15, 16 },
        }}),
        // Signal 'domainEvent'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &, const QString &, const QVariantMap &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 }, { QMetaType::QString, 18 }, { QMetaType::QString, 19 }, { QMetaType::QString, 20 },
            { 0x80000000 | 21, 22 },
        }}),
        // Signal 'providerError'
        QtMocHelpers::SignalData<void(const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 24 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AgentProvider, qt_meta_tag_ZN7Artemis13AgentProviderE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Artemis::AgentProvider::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AgentProviderE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AgentProviderE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7Artemis13AgentProviderE_t>.metaTypes,
    nullptr
} };

void Artemis::AgentProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AgentProvider *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->readyChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->setupChanged(); break;
        case 2: _t->versionChanged(); break;
        case 3: _t->activeTurnStarted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->tokenUsageUpdated((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[4]))); break;
        case 5: _t->userInputRequested((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[4]))); break;
        case 6: _t->domainEvent((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QVariantMap>>(_a[5]))); break;
        case 7: _t->providerError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(bool )>(_a, &AgentProvider::readyChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)()>(_a, &AgentProvider::setupChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)()>(_a, &AgentProvider::versionChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(const QString & , const QString & )>(_a, &AgentProvider::activeTurnStarted, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(const QString & , qint64 , qint64 , qint64 )>(_a, &AgentProvider::tokenUsageUpdated, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(const QString & , const QString & , const QString & , const QVariantList & )>(_a, &AgentProvider::userInputRequested, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(const QString & , const QString & , const QString & , const QString & , const QVariantMap & )>(_a, &AgentProvider::domainEvent, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (AgentProvider::*)(const QString & )>(_a, &AgentProvider::providerError, 7))
            return;
    }
}

const QMetaObject *Artemis::AgentProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Artemis::AgentProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7Artemis13AgentProviderE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Artemis::AgentProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Artemis::AgentProvider::readyChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void Artemis::AgentProvider::setupChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Artemis::AgentProvider::versionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Artemis::AgentProvider::activeTurnStarted(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void Artemis::AgentProvider::tokenUsageUpdated(const QString & _t1, qint64 _t2, qint64 _t3, qint64 _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 5
void Artemis::AgentProvider::userInputRequested(const QString & _t1, const QString & _t2, const QString & _t3, const QVariantList & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 6
void Artemis::AgentProvider::domainEvent(const QString & _t1, const QString & _t2, const QString & _t3, const QString & _t4, const QVariantMap & _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 7
void Artemis::AgentProvider::providerError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}
QT_WARNING_POP
