#include "gpurastereffect.h"
#include "exceptions.h"
#include "Animators/qrealanimator.h"
#include "Animators/gpueffectanimators.h"
#include "gpurastereffectcreator.h"

GPURasterEffect::GPURasterEffect(const GPURasterEffectCreator * const creator,
                                 const GPURasterEffectProgram * const program,
                                 const QString& name) :
    ComplexAnimator(name), mProgram(program), mCreator(creator) {

}

void GPURasterEffect::writeProperty(QIODevice * const dst) const {
    for(const auto& anim : ca_mChildAnimators)
        anim->writeProperty(dst);
}

void GPURasterEffect::readProperty(QIODevice * const src) {
    for(const auto& anim : ca_mChildAnimators)
        anim->readProperty(src);
}

void GPURasterEffect::writeIdentifier(QIODevice * const dst) const {
    mCreator->writeIdentifier(dst);
}
