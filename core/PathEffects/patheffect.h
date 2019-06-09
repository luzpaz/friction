#ifndef PATHEFFECT_H
#define PATHEFFECT_H
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"
#include "Animators/staticcomplexanimator.h"
class BoolProperty;
class PathBox;
class BoundingBox;
enum PathEffectType : short {
    DISPLACE_PATH_EFFECT,
    DASH_PATH_EFFECT,
    DUPLICATE_PATH_EFFECT,
    SOLIDIFY_PATH_EFFECT,
    OPERATION_PATH_EFFECT,
    LENGTH_PATH_EFFECT,
    SUM_PATH_EFFECT,
    SUB_PATH_EFFECT,
    LINES_PATH_EFFECT,
    ZIGZAG_PATH_EFFECT,
    SPATIAL_DISPLACE_PATH_EFFECT,
    SUBDIVIDE_PATH_EFFECT,
    CUSTOM_PATH_EFFECT
};
class PathEffect;

typedef PropertyMimeData<PathEffect,
    InternalMimeData::PATH_EFFECT> PathEffectMimeData;

class PathEffect : public StaticComplexAnimator {
protected:
    PathEffect(const QString& name,
               const PathEffectType type);
public:
    virtual void apply(const qreal relFrame,
                       const SkPath &src,
                       SkPath * const dst) = 0;
    virtual bool hasReasonsNotToApplyUglyTransform();

    bool SWT_isPathEffect() const;
    QMimeData *SWT_createMimeData();
    void prp_startDragging();

    void writeIdentifier(QIODevice * const dst) const;
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);

    PathEffectType getEffectType();

    void switchVisible();
    void setVisible(const bool bT);
    bool isVisible() const;
protected:
    bool mVisible = true;
    PathEffectType mPathEffectType;
};

#endif // PATHEFFECT_H
