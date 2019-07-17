#include "drawableautotiledsurface.h"
#include "castmacros.h"
#include "skia/skiahelpers.h"

DrawableAutoTiledSurface::DrawableAutoTiledSurface() :
    mRowCount(mTileImgs.fRowCount),
    mColumnCount(mTileImgs.fColumnCount),
    mZeroTileRow(mTileImgs.fZeroTileRow),
    mZeroTileCol(mTileImgs.fZeroTileCol),
    mImgs(mTileImgs.fImgs) {}

void DrawableAutoTiledSurface::drawOnCanvas(SkCanvas * const canvas,
                                            GrContext* const grContext,
                                            const SkPoint &dst,
                                            const QRect * const minPixSrc,
                                            SkPaint * const paint) const {
    const QRect maxRect = tileBoundingRect();
    QRect tileRect;
    if(minPixSrc) {
        const QRect tileSrc = pixRectToTileRect(*minPixSrc);
        if(!tileSrc.intersects(maxRect)) return;
        tileRect = tileSrc.intersected(maxRect);
    } else tileRect = maxRect;
    for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
        const SkScalar drawX = dst.x() + tx*TILE_SIZE;
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            const auto img = imageForTile(tx, ty);
            if(!img) continue;
            const SkScalar drawY = dst.y() + ty*TILE_SIZE;
            if(grContext) {
                SkiaHelpers::drawImageGPU(canvas, img, drawX, drawY,
                                          paint, grContext);
            } else canvas->drawImage(img, drawX, drawY, paint);
        }
    }
}

using namespace std;
using namespace std::chrono;
void DrawableAutoTiledSurface::updateTileRectImgs(QRect tileRect) {
    const QRect maxRect = mSurface.tileBoundingRect();
    if(!maxRect.intersects(tileRect)) return;
    tileRect = maxRect.intersected(tileRect);
    const auto min = tileRect.topLeft();
    const auto max = tileRect.bottomRight();
    stretchToTileImg(min.x(), min.y());
    stretchToTileImg(max.x(), max.y());
    const int n = tileRect.width()*tileRect.height();
    #pragma omp parallel for collapse(2) if(n > 4)
    for(int tx = tileRect.left(); tx <= tileRect.right(); tx++) {
        for(int ty = tileRect.top(); ty <= tileRect.bottom(); ty++) {
            auto btmp = mSurface.tileToBitmap(tx, ty);
            const auto img = SkiaHelpers::transferDataToSkImage(btmp);

            const auto tileId = QPoint(tx, ty) + zeroTile();
            mImgs[tileId.x()].replace(tileId.y(), img);
        }
    }
}

class TilesTmpFileDataSaver : public TmpFileDataSaver {
    friend class StdSelfRef;
public:
    typedef std::function<void(const qsptr<QTemporaryFile>&)> Func;
protected:
    TilesTmpFileDataSaver(const TileImgs &images,
                          const Func& finishedFunc) :
        mImages(images), mFinishedFunc(finishedFunc) {}

    void writeToFile(QIODevice * const file) {
        file->write(rcConstChar(&mImages.fRowCount), sizeof(int));
        file->write(rcConstChar(&mImages.fColumnCount), sizeof(int));
        file->write(rcConstChar(&mImages.fZeroTileRow), sizeof(int));
        file->write(rcConstChar(&mImages.fZeroTileCol), sizeof(int));
        for(const auto& col : mImages.fImgs) {
            for(const auto& tile : col)
                SkiaHelpers::writeImg(tile, file);
        }
    }

    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mTmpFile);
    }
private:
    const TileImgs mImages;
    const Func mFinishedFunc;
};

class TilesTmpFileDataLoader : public TmpFileDataLoader {
    friend class StdSelfRef;
public:
    typedef std::function<void(const TileImgs&)> Func;
protected:
    TilesTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                           const Func& finishedFunc) :
        TmpFileDataLoader(file), mFinishedFunc(finishedFunc) {}

    void readFromFile(QIODevice * const file) {
        file->read(rcChar(&mTileImgs.fRowCount), sizeof(int));
        file->read(rcChar(&mTileImgs.fColumnCount), sizeof(int));
        file->read(rcChar(&mTileImgs.fZeroTileRow), sizeof(int));
        file->read(rcChar(&mTileImgs.fZeroTileCol), sizeof(int));
        for(int i = 0; i < mTileImgs.fColumnCount; i++) {
            mTileImgs.fImgs.append(QList<sk_sp<SkImage>>());
            auto& col = mTileImgs.fImgs.last();
            for(int j = 0; j < mTileImgs.fZeroTileRow; j++)
                col.append(SkiaHelpers::readImg(file));
        }
    }
    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mTileImgs);
    }
private:
    TileImgs mTileImgs;
    const Func mFinishedFunc;
};


stdsptr<HDDTask> DrawableAutoTiledSurface::createTmpFileDataSaver() {
    const TilesTmpFileDataSaver::Func func =
            [this](const qsptr<QTemporaryFile>& tmpFile) {
        setDataSavedToTmpFile(tmpFile);
    };
    return SPtrCreate(TilesTmpFileDataSaver)(mTileImgs, func);
}

stdsptr<HDDTask> DrawableAutoTiledSurface::createTmpFileDataLoader() {
    const TilesTmpFileDataLoader::Func func =
            [this](const TileImgs& tiles) {
        setTileImgs(tiles);
    };
    return SPtrCreate(TilesTmpFileDataLoader)(mTmpFile, func);
}
