#ifndef DRAWABLEAUTOTILEDSURFACE_H
#define DRAWABLEAUTOTILEDSURFACE_H
#include "autotiledsurface.h"
#include "skia/skiahelpers.h"
#include "CacheHandlers/hddcachablecont.h"

struct TileBitmaps {
    int fRowCount = 0;
    int fColumnCount = 0;
    int fZeroTileRow = 0;
    int fZeroTileCol = 0;
    QList<QList<SkBitmap>> fBitmaps;

    void deepCopy(const TileBitmaps& src) {
        fRowCount = src.fRowCount;
        fColumnCount = src.fColumnCount;
        fZeroTileRow = src.fZeroTileRow;
        fZeroTileCol = src.fZeroTileCol;
        fBitmaps.clear();
        for(const auto& srcList : src.fBitmaps) {
            fBitmaps << QList<SkBitmap>();
            auto& list = fBitmaps.last();
            for(const auto& srcBitmap : srcList) {
                list << SkiaHelpers::makeCopy(srcBitmap);
            }
        }
    }

    bool isEmpty() const { return fBitmaps.isEmpty(); }

    void clear() {
        fBitmaps.clear();
        fZeroTileCol = 0;
        fZeroTileRow = 0;
        fColumnCount = 0;
        fRowCount = 0;
    }
};

class DrawableAutoTiledSurface : public HDDCachablePersistent {
    e_OBJECT
    typedef QList<QList<SkBitmap>> Tiles;
protected:
    DrawableAutoTiledSurface();
    DrawableAutoTiledSurface(const DrawableAutoTiledSurface& other) = delete;
    DrawableAutoTiledSurface& operator=(const DrawableAutoTiledSurface& other) = delete;

    stdsptr<HDDTask> createTmpFileDataSaver();

    stdsptr<HDDTask> createTmpFileDataLoader();

    int getByteCount() {
        const int spixels = mColumnCount*mRowCount*TILE_SPIXEL_SIZE;
        return spixels*static_cast<int>(sizeof(uint16_t));
    }

    int clearMemory() {
        return 0;
        const int bytes = getByteCount();
        clearBitmaps();
        return bytes;
    }
public:
    void deepCopy(const DrawableAutoTiledSurface& other) {
        mSurface.deepCopy(other.mSurface);
        mTileBitmaps.deepCopy(other.mTileBitmaps);
    }

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      const QRect * const minPixSrc,
                      SkPaint * const paint) const;

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      const QRect * const minPixSrc) const {
        drawOnCanvas(canvas, dst, minPixSrc, nullptr);
    }

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst) const {
        drawOnCanvas(canvas, dst, nullptr, nullptr);
    }

    void drawOnCanvas(SkCanvas * const canvas,
                      const SkPoint &dst,
                      SkPaint * const paint) const {
        drawOnCanvas(canvas, dst, nullptr, paint);
    }

    const AutoTiledSurface &surface() const {
        return mSurface;
    }

    void pixelRectChanged(const QRect& pixRect) {
        if(mTmpFile) scheduleDeleteTmpFile();
        updateTileRecBitmaps(pixRectToTileRect(pixRect));
    }

    QRect pixelBoundingRect() const {
        return tileRectToPixRect(tileBoundingRect());
    }

    int width() const {
        return pixelBoundingRect().width();
    }

    int height() const {
        return pixelBoundingRect().height();
    }

    void write(QIODevice *target) {
        mSurface.write(target);
    }

    void read(QIODevice *target) {
        mSurface.read(target);
        updateTileBitmaps();
    }
private:
    void updateTileBitmaps() {
        updateTileRecBitmaps(mSurface.tileBoundingRect());
    }

    void updateTileRecBitmaps(QRect tileRect);

    void setTileBitmaps(const TileBitmaps& tiles) {
        mTileBitmaps = tiles;
    }

    void clearBitmaps() {
        mTileBitmaps.clear();
    }

    void stretchBitmapsToTile(const int tx, const int ty) {
        const int colId = tx + mZeroTileCol;
        const int rowId = ty + mZeroTileRow;

        if(rowId < 0) {
            prependBitmapRows(qAbs(rowId));
        } else if(rowId >= mRowCount) {
            appendBitmapRows(qAbs(rowId - mRowCount + 1));
        }
        if(colId < 0) {
            prependBitmapColumns(qAbs(colId));
        } else if(colId >= mColumnCount) {
            appendBitmapColumns(qAbs(colId - mColumnCount + 1));
        }
    }

    QList<SkBitmap> newBitmapColumn() {
        QList<SkBitmap> col;
        for(int j = 0; j < mRowCount; j++) col.append(SkBitmap());
        return col;
    }

    void prependBitmapRows(const int count) {
        for(auto& col : mBitmaps) {
            for(int i = 0; i < count; i++) {
                col.prepend(SkBitmap());
            }
        }
        mRowCount += count;
        mZeroTileRow += count;
    }

    void appendBitmapRows(const int count) {
        for(auto& col : mBitmaps) {
            for(int i = 0; i < count; i++) {
                col.append(SkBitmap());
            }
        }
        mRowCount += count;
    }

    void prependBitmapColumns(const int count) {
        for(int i = 0; i < count; i++) {
            mBitmaps.prepend(newBitmapColumn());
        }
        mColumnCount += count;
        mZeroTileCol += count;
    }

    void appendBitmapColumns(const int count) {
        for(int i = 0; i < count; i++) {
            mBitmaps.append(newBitmapColumn());
        }
        mColumnCount += count;
    }

    SkBitmap bitmapForTile(const int tx, const int ty) const {
        const auto zeroTileV = zeroTile();
        return imageForTileId(tx + zeroTileV.x(), ty + zeroTileV.y());
    }

    SkBitmap imageForTileId(const int colId, const int rowId) const {
        return mBitmaps.at(colId).at(rowId);
    }

    QPoint zeroTile() const {
        return QPoint(mZeroTileCol, mZeroTileRow);
    }

    QPoint zeroTilePos() const {
        return zeroTile()*TILE_SIZE;
    }

    QRect tileBoundingRect() const {
        return QRect(-mZeroTileCol, -mZeroTileRow,
                     mColumnCount, mRowCount);
    }

    QRect tileRectToPixRect(const QRect& tileRect) const {
        return QRect(tileRect.x()*TILE_SIZE,
                     tileRect.y()*TILE_SIZE,
                     tileRect.width()*TILE_SIZE,
                     tileRect.height()*TILE_SIZE);
    }

    QRect pixRectToTileRect(const QRect& pixRect) const {
        const int widthRem = (pixRect.width() % TILE_SIZE) ? 2 : 1;
        const int heightRem = (pixRect.height() % TILE_SIZE) ? 2 : 1;
        return QRect(qFloor(static_cast<qreal>(pixRect.x())/TILE_SIZE),
                     qFloor(static_cast<qreal>(pixRect.y())/TILE_SIZE),
                     pixRect.width()/TILE_SIZE + widthRem,
                     pixRect.height()/TILE_SIZE + heightRem);
    }

    AutoTiledSurface mSurface;
    TileBitmaps mTileBitmaps;
    int &mRowCount;
    int &mColumnCount;
    int &mZeroTileRow;
    int &mZeroTileCol;
    Tiles &mBitmaps;
};

#endif // DRAWABLEAUTOTILEDSURFACE_H