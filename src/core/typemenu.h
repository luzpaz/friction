﻿#ifndef TYPEMENU_H
#define TYPEMENU_H

#include <QMenu>
#include <typeindex>
#include "smartPointers/sharedpointerdefs.h"
#include "canvasbase.h"

class Property;
class MovablePoint;
class BoundingBox;

template <typename Type>
class TypeMenu {
    friend class Canvas;
    using TTypeMenu = TypeMenu<Type>;
public:
    using PlainTriggeredOp = std::function<void()>;
    using CheckTriggeredOp = std::function<void(bool)>;
    template <class T> using PlainSelectedOp = std::function<void(T*)>;
    template <class T> using CheckSelectedOp = std::function<void(T*, bool)>;
    template <class T> using AllOp = std::function<void(const QList<T*>&)>;

    TypeMenu(QMenu * const targetMenu,
             CanvasBase * const targetCanvas,
             QWidget * const parent) :
        mQMenu(targetMenu),
        mTargetCanvas(targetCanvas),
        mParentWidget(parent) {}

    QAction* addSection(const QString& name) {
        return mQMenu->addSection(name);
    }

    QAction* addCheckableAction(const QString& text, const bool checked,
                                const CheckTriggeredOp& op) {
        QAction * const qAction = mQMenu->addAction(text);
        qAction->setCheckable(true);
        qAction->setChecked(checked);
        QObject::connect(qAction, &QAction::triggered, op);
        return qAction;
    }

    template <class T>
    QAction* addCheckableAction(const QString& text, const bool checked,
                                const CheckSelectedOp<T>& op) {
        QAction * const qAction = mQMenu->addAction(text);
        qAction->setCheckable(true);
        qAction->setChecked(checked);
        const PlainSelectedOp<T> plainOp = [op, checked](T* pt) {
            op(pt, !checked);
        };
        connectAction(qAction, plainOp);
        return qAction;
    }

    QAction* addPlainAction(const QString& text, const PlainTriggeredOp& op) {
        QAction * const qAction = mQMenu->addAction(text);
        QObject::connect(qAction, &QAction::triggered, op);
        return qAction;
    }

    template <class T>
    QAction* addPlainAction(const QString& text, const PlainSelectedOp<T>& op) {
        QAction * const qAction = mQMenu->addAction(text);
        connectAction(qAction, op);
        return qAction;
    }

    template <class T>
    QAction* addPlainAction(const QString& text, const AllOp<T>& op) {
        QAction * const qAction = mQMenu->addAction(text);
        connectAction(qAction, op);
        return qAction;
    }

    TTypeMenu * addMenu(const QString& title) {
        QMenu * const qMenu = mQMenu->addMenu(title);
        const auto child = std::make_shared<TTypeMenu>(qMenu, mTargetCanvas,
                                                       mParentWidget);
        mChildMenus.append(child);
        return child.get();
    }

    QAction* addSeparator() {
        return mQMenu->addSeparator();
    }

    bool isEmpty() {
        return mQMenu->isEmpty();
    }

    void clear() {
        mQMenu->clear();
        mChildMenus.clear();
        mTypeIndex.clear();
    }

    QWidget* getParentWidget() const {
        return mParentWidget;
    }

    void addSharedMenu(const QString& name) {
        mSharedMenus << name;
    }

    bool hasSharedMenu(const QString& name) const {
        return mSharedMenus.contains(name);
    }
protected:
    void addedActionsForType(Type * const obj) {
        mTypeIndex.append(std::type_index(typeid(obj)));
    }

    bool hasActionsForType(Type * const obj) const {
        return mTypeIndex.contains(std::type_index(typeid(obj)));
    }
private:
    template <typename U>
    void connectAction(BoundingBox * const, QAction * const qAction,
                       const U& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedBoxes(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    template <typename U>
    void connectAction(MovablePoint * const, QAction * const qAction,
                       const U& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedPoints(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    template <typename U>
    void connectAction(Property * const, QAction * const qAction,
                       const U& op) {
        const auto targetCanvas = mTargetCanvas;
        const auto canvasOp = [op, targetCanvas]() {
            targetCanvas->execOpOnSelectedProperties(op);
        };
        QObject::connect(qAction, &QAction::triggered, canvasOp);
    }

    template <class T>
    void connectAction(QAction * const qAction, const PlainSelectedOp<T>& op) {
        connectAction(static_cast<T*>(nullptr), qAction, op);
    }

    template <class T>
    void connectAction(QAction * const qAction, const AllOp<T>& op) {
        connectAction(static_cast<T*>(nullptr), qAction, op);
    }

    QMenu * const mQMenu;
    CanvasBase * const mTargetCanvas;
    QWidget * const mParentWidget;

    QList<stdsptr<TTypeMenu>> mChildMenus;
    QList<std::type_index> mTypeIndex;
    QStringList mSharedMenus;
};

typedef TypeMenu<MovablePoint> PointTypeMenu;
typedef TypeMenu<Property> PropertyMenu;

#endif // TYPEMENU_H