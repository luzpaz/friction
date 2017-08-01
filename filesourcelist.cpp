#include "filesourcelist.h"
#include "global.h"
#include <QPainter>
#include <QScrollBar>
#include <QMenu>
#include "mainwindow.h"

FileSourceWidget::FileSourceWidget(FileSourceListVisibleWidget *parent) :
    QWidget(parent) {
    mParentVisibleWidget = parent;
    setFixedHeight(MIN_WIDGET_HEIGHT);
}

void FileSourceWidget::setTargetCache(FileCacheHandlerAbstraction *target) {
    mTargetCache = target;
    if(mTargetCache == NULL) {
        setToolTip("");
    } else {
        setToolTip(mTargetCache->getFilePath());
    }
}

void FileSourceWidget::paintEvent(QPaintEvent *) {
    if(mTargetCache == NULL || width() <= 2*MIN_WIDGET_HEIGHT) return;
    QPainter p(this);

    QString wholeString = mTargetCache->getFilePath();
    if(mFileNameOnly) {
        wholeString = wholeString.split("/").last();
    }
    int wholeWidth = p.fontMetrics().width(wholeString);
    bool addDots = false;
    while(wholeWidth > width() - 1.5*MIN_WIDGET_HEIGHT) {
        addDots = true;
        int guessLen =
                (width() - 1.5*MIN_WIDGET_HEIGHT -
                 p.fontMetrics().width("..."))*
                wholeString.count()/wholeWidth;
        wholeString = wholeString.right(guessLen);
        wholeWidth = p.fontMetrics().width("..." + wholeString);
    }
    if(addDots) {
        wholeString = "..." + wholeString;
    }

    if(mTargetCache->selected) {
        p.fillRect(QRect(0.5*MIN_WIDGET_HEIGHT, 0,
                         wholeWidth + MIN_WIDGET_HEIGHT,
                         MIN_WIDGET_HEIGHT),
                   QColor(180, 180, 180));
        p.setPen(Qt::black);
    }

    p.drawText(rect().adjusted(MIN_WIDGET_HEIGHT, 0,
                               -0.5*MIN_WIDGET_HEIGHT, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               wholeString);

    p.end();
}

void FileSourceWidget::switchFileNameOnly() {
    mFileNameOnly = !mFileNameOnly;
}

void FileSourceWidget::mouseReleaseEvent(QMouseEvent *event) {
    if(mTargetCache == NULL) return;
    if(event->button() == Qt::LeftButton) {
        if(event->modifiers() & Qt::ShiftModifier) {
            mTargetCache->switchSelected();
        } else {
            mParentVisibleWidget->clearSelected();
            mTargetCache->setSelected(true);
        }
    } else {
        if(!mTargetCache->selected) {
            mParentVisibleWidget->clearSelected();
            mTargetCache->setSelected(true);
        }
        mParentVisibleWidget->showContextMenu(event->globalPos());
    }

    update();
}

FileSourceListScrollWidget::FileSourceListScrollWidget(ScrollArea *parent) :
    MinimalScrollWidget(parent) {
    createVisiblePartWidget();
    updateHeight();
}

void FileSourceListScrollWidget::updateHeight() {
    setFixedHeight((FileSourcesCache::getFileCacheList().count() + 0.5) *
                   MIN_WIDGET_HEIGHT);
}

void FileSourceListScrollWidget::createVisiblePartWidget() {
    mMinimalVisiblePartWidget = new FileSourceListVisibleWidget(this);
}

FileSourceListVisibleWidget::FileSourceListVisibleWidget(MinimalScrollWidget *parent) :
    MinimalScrollWidgetVisiblePart(parent) {
    FileSourcesCache::addFileSourceListVisibleWidget(this);
}

void FileSourceListVisibleWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);

    int currY = MIN_WIDGET_HEIGHT;
    p.setPen(QPen(QColor(40, 40, 40), 1.));
    while(currY < height()) {
        p.drawLine(0, currY, width(), currY);

        currY += MIN_WIDGET_HEIGHT;
    }

//    if(mDragging) {
//        p.setPen(QPen(Qt::white, 3.));
//        p.drawLine(0, mCurrentDragPosId*MIN_WIDGET_HEIGHT,
//                   width(), mCurrentDragPosId*MIN_WIDGET_HEIGHT);
//    }

    p.end();
}

FileSourceListVisibleWidget::~FileSourceListVisibleWidget() {
    FileSourcesCache::removeFileSourceListVisibleWidget(this);
}

void FileSourceListVisibleWidget::updateVisibleWidgetsContent() {
    int firstVisibleId = mVisibleTop/MIN_WIDGET_HEIGHT;

    int idP = 0;
    for(int i = firstVisibleId;
        i < mCacheList.count() &&
        idP < mSingleWidgets.count(); i++) {
        FileSourceWidget *fsw = ((FileSourceWidget*)mSingleWidgets.at(idP));
        fsw->setTargetCache(mCacheList.at(i));
        fsw->show();
        idP++;
    }

    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

QWidget *FileSourceListVisibleWidget::createNewSingleWidget() {
    return new FileSourceWidget(this);
}

void FileSourceListVisibleWidget::addCacheHandlerToList(
        FileCacheHandler *handler) {
    mCacheList << new FileCacheHandlerAbstraction(handler, this);
    scheduleContentUpdate();
}

void FileSourceListVisibleWidget::removeCacheHandlerFromList(
        FileCacheHandler *handler) {
    for(int i = 0; i < mCacheList.count(); i++) {
        FileCacheHandlerAbstraction *abs = mCacheList.at(i);
        if(abs->target == handler) {
            if(abs->selected) {
                removeFromSelectedList(abs);
            }
            mCacheList.removeAt(i);
            scheduleContentUpdate();
            return;
        }
    }
}

void FileSourceListVisibleWidget::showContextMenu(const QPoint &globalPos) {
    QMenu menu;
    menu.addAction("reload");
    if(mSelectedList.count() == 1) {
        menu.addAction("replace...");
    }
    QAction *selected_action = menu.exec(globalPos);
    if(selected_action != NULL) {
        if(selected_action->text() == "reload") {
            foreach(FileCacheHandlerAbstraction *abs, mSelectedList) {
                abs->target->clearCache();
            }
        } else if(selected_action->text() == "replace...") {
            mSelectedList.first()->target->replace();
        }

        MainWindow::getInstance()->callUpdateSchedulers();
    } else {

    }
}

FileSourceList::FileSourceList(QWidget *parent) : ScrollArea(parent) {
    mScrollWidget = new FileSourceListScrollWidget(this);
    setWidget(mScrollWidget);

    connect(verticalScrollBar(),
            SIGNAL(valueChanged(int)),
            mScrollWidget, SLOT(changeVisibleTop(int)));
    connect(this, SIGNAL(heightChanged(int)),
            mScrollWidget, SLOT(changeVisibleHeight(int)));
    connect(this, SIGNAL(widthChanged(int)),
            mScrollWidget, SLOT(setWidth(int)));

    verticalScrollBar()->setSingleStep(
                MIN_WIDGET_HEIGHT);
}

void FileCacheHandlerAbstraction::setSelected(const bool &bT) {
    if(bT == selected) return;
    selected = bT;
    if(selected) {
        parentVisibleWidget->addToSelectedList(this);
    } else {
        parentVisibleWidget->removeFromSelectedList(this);
    }
}
