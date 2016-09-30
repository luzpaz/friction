#ifndef VECTORPATH_H
#define VECTORPATH_H
#include <QPainterPath>
#include "boundingbox.h"
#include "pathpoint.h"
#include <QLinearGradient>
#include "gradientpoint.h"
#include "pathanimator.h"

class BoxesGroup;

class MainWindow;

class PathPivot;

enum CanvasMode : short;

class VectorPath;

class Edge;

class GradientPoints {
public:
    GradientPoints();

    void initialize(VectorPath *parentT, QPointF startPt = QPointF(0.f, 0.f),
                    QPointF endPt = QPointF(100.f, 100.f));

    void initialize(VectorPath *parentT,
                    int fillGradientStartId, int fillGradientEndId);

    void clearAll();

    void enable();

    void disable();

    void draw(QPainter *p);

    MovablePoint *getPointAt(QPointF absPos);

    QPointF getStartPoint();

    QPointF getEndPoint();

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    VectorPath *parent;

    void attachToBoneFromSqlZId();

    void setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo = true);
};

class VectorPath : public BoundingBox
{
public:
    VectorPath(BoxesGroup *group);
    VectorPath(int boundingBoxId, BoxesGroup *parent);

    virtual QRectF getBoundingRect();
    void draw(QPainter *p);
    void render(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);

    PathPoint *addPointAbsPos(QPointF absPtPos, PathPoint *toPoint = NULL);
    PathPoint *addPointRelPos(QPointF relPtPos, PathPoint *toPoint = NULL);

    bool pointInsidePath(QPointF point);
    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);

    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint*> *list);

    PathPoint *addPoint(PathPoint *pointToAdd, PathPoint *toPoint);

    void removePoint(PathPoint *point);
    void replaceSeparatePathPoint(PathPoint *pointBeingReplaced, PathPoint *newPoint);
    void addPointToSeparatePaths(PathPoint *pointToAdd, bool saveUndoRedo = true);
    void removePointFromSeparatePaths(PathPoint *pointToRemove, bool saveUndoRedo = true);
    void appendToPointsList(PathPoint *point, bool saveUndoRedo = true);
    void removeFromPointsList(PathPoint *point, bool saveUndoRedo = true);

    void schedulePathUpdate();
    void scheduleMappedPathUpdate();

    void updatePathIfNeeded();
    void updateMappedPathIfNeeded();
    void centerPivotPosition();

    void resetStrokeGradientPointsPos(bool finish) {
        mStrokeGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
    }

    void resetFillGradientPointsPos(bool finish) {
        mFillGradientPoints.setPositions(getBoundingRect().topLeft(),
                     getBoundingRect().bottomRight(), finish);
    }

    void changeFillGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mFillPaintSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetFillGradientPointsPos(finish);
        }
    }

    void changeStrokeGradient(bool wasGradient, bool isGradient, bool finish) {
        if(wasGradient) {
            mStrokeSettings.getGradient()->removePath(this);
        } else if(isGradient) {
            resetStrokeGradientPointsPos(finish);
        }
    }

    virtual void setFillGradient(Gradient* gradient, bool finish) {
        if(mFillPaintSettings.getPaintType() != GRADIENTPAINT) {
            setFillPaintType(GRADIENTPAINT,
                              mFillPaintSettings.getCurrentColor(),
                              gradient);
        } else {
            changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mFillPaintSettings.setGradient(gradient);
            updateDrawGradients();


        }
    }

    virtual void setStrokeGradient(Gradient* gradient, bool finish) {
        if(mStrokeSettings.getPaintType() != GRADIENTPAINT) {
            setStrokePaintType(GRADIENTPAINT,
                               mStrokeSettings.getCurrentColor(),
                               gradient);
        } else {
            changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                                 true, finish);

            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();


        }
    }

    virtual void setFillFlatColor(Color color, bool finish) {
        if(mFillPaintSettings.getPaintType() != FLATPAINT) {
            setFillPaintType(FLATPAINT, color, NULL);
        } else {
            mFillPaintSettings.setCurrentColor(color);
            if(finish) {
                mFillPaintSettings.getColorAnimator()->finishTransform();
            }


        }
    }

    virtual void setStrokeFlatColor(Color color, bool finish) {
        if(mStrokeSettings.getPaintType() != FLATPAINT) {
            setStrokePaintType(FLATPAINT, color, NULL);
        } else {
            mStrokeSettings.setCurrentColor(color);
            if(finish) {
                mStrokeSettings.getColorAnimator()->finishTransform();
            }


        }
    }

    virtual void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient) {
        changeFillGradient(mFillPaintSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mFillPaintSettings.setGradient(gradient);
            updateDrawGradients();
        } else if(paintType == FLATPAINT) {
            mFillPaintSettings.setCurrentColor(color);
        }
        mFillPaintSettings.setPaintType(paintType);


    }

    virtual void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient) {
        changeStrokeGradient(mStrokeSettings.getPaintType() == GRADIENTPAINT,
                           paintType == GRADIENTPAINT, true);
        if(paintType == GRADIENTPAINT) {
            mStrokeSettings.setGradient(gradient);
            updateDrawGradients();
        } else if(paintType == FLATPAINT) {
            mStrokeSettings.setCurrentColor(color);
        }
        mStrokeSettings.setPaintType(paintType);


    }

    virtual void setStrokeCapStyle(Qt::PenCapStyle capStyle) {
        mStrokeSettings.setCapStyle(capStyle);
        updateOutlinePath();


    }

    virtual void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle) {
        mStrokeSettings.setJoinStyle(joinStyle);
        updateOutlinePath();

    }

    virtual void setStrokeWidth(qreal strokeWidth, bool finish) {
        mStrokeSettings.setCurrentStrokeWidth(strokeWidth);
        if(finish) {
            mStrokeSettings.getStrokeWidthAnimator()->finishTransform();
        }
        updateOutlinePath();


    }

    void startStrokeWidthTransform() {
        mStrokeSettings.getStrokeWidthAnimator()->startTransform();
    }

    void startStrokeColorTransform() {
        mStrokeSettings.getColorAnimator()->startTransform();
    }

    void startFillColorTransform() {
        mFillPaintSettings.getColorAnimator()->startTransform();
    }

    const StrokeSettings *getStrokeSettings();
    const PaintSettings *getFillSettings();
    void updateDrawGradients();

    PathPoint *addPointRelPos(QPointF relPos,
                              QPointF startRelPos, QPointF endRelPos,
                              PathPoint *toPoint = NULL);
    int saveToSql(int parentId);

    void clearAll();

    PathPoint *createNewPointOnLineNear(QPointF absPos);
    qreal percentAtPoint(QPointF absPos, qreal distTolerance,
                         qreal maxPercent, qreal minPercent,
                         bool *found = NULL, QPointF *posInPath = NULL);
    PathPoint *findPointNearestToPercent(qreal percent, qreal *foundAtPercent);

    void attachToBoneFromSqlZId();

    void updateAfterFrameChanged(int currentFrame);

    void startAllPointsTransform();
    void finishAllPointsTransform();

    Edge *getEgde(QPointF absPos);
protected:
    PathAnimator mPathAnimator;

    void loadPointsFromSql(int vectorPathId);

    GradientPoints mFillGradientPoints;
    GradientPoints mStrokeGradientPoints;

    QLinearGradient mDrawFillGradient;
    QLinearGradient mDrawStrokeGradient;
    QPen mDrawPen;

    PaintSettings mFillPaintSettings;
    StrokeSettings mStrokeSettings;

    void updatePath();
    void updateMappedPath();

    bool mPathUpdateNeeded = false;
    bool mMappedPathUpdateNeeded = false;

    bool mClosedPath = false;
    QList<PathPoint*> mSeparatePaths;
    QList<PathPoint*> mPoints;
    QPainterPath mPath;
    QPainterPath mMappedPath;
    QPainterPath mOutlinePath;
    QPainterPathStroker mPathStroker;
    QPainterPath mMappedWhole;
    void updateOutlinePath();
    void updateWholePath();
    qreal findPercentForPoint(QPointF point, qreal minPercent = 0.f,
                              qreal maxPercent = 1.f);
protected:
    void updateAfterTransformationChanged();
    void updateAfterCombinedTransformationChanged();
};

#endif // VECTORPATH_H
