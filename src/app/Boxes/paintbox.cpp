#include "paintbox.h"
#include "canvas.h"
#include "MovablePoints/animatedpoint.h"
#include "Animators/transformanimator.h"

PaintBox::PaintBox() : BoundingBox(TYPE_PAINT) {
    prp_setName("Paint Box");
    mSurface = SPtrCreate(AnimatedSurface)();
    ca_addChildAnimator(mSurface);
}

void PaintBox::setupRenderData(
        const qreal relFrame, BoxRenderData * const data) {
    BoundingBox::setupRenderData(relFrame, data);
    auto paintData = GetAsSPtr(data, PaintBoxRenderData);
    paintData->fSurface = GetAsSPtr(mSurface->getSurface(qFloor(relFrame)),
                                    DrawableAutoTiledSurface);
}

stdsptr<BoxRenderData> PaintBox::createRenderData() {
    return SPtrCreate(PaintBoxRenderData)(this);
}

#include <QFileDialog>
#include "typemenu.h"
void PaintBox::setupCanvasMenu(PropertyMenu * const menu) {
    const auto widget = menu->getParentWidget();
    PropertyMenu::PlainSelectedOp<PaintBox> op = [widget](PaintBox * box) {
        const QString importPath = QFileDialog::getOpenFileName(
                                        widget,
                                        "Load From Image", "",
                                        "Image Files (*.png *.jpg)");
        if(!importPath.isEmpty()) {
            QImage img;
            if(img.load(importPath)) {
                //box->loadFromImage(img);
            }
        }
    };
    menu->addPlainAction("Load From Image", op);

    BoundingBox::setupCanvasMenu(menu);
}