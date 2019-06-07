#ifndef GPURASTEREFFECTCREATOR_H
#define GPURASTEREFFECTCREATOR_H
#include "gpurastereffectprogram.h"

enum PropertyType {
    PTYPE_FLOAT,
    PTYPE_INT
};

struct GPURasterEffectCreator : public PropertyCreator {
protected:
    GPURasterEffectCreator(const QString& grePath, const QString& name,
                           const QList<stdsptr<PropertyCreator>>& propCs,
                           const GPURasterEffectProgram& program) :
        PropertyCreator(name), fGrePath(grePath),
        fProperties(propCs), fProgram(program) {}
public:
    friend class StdSelfRef;

    struct Identifier {
        Identifier(const QString& grePath, const QString& name,
                   const QList<PropertyType>& types) :
        fGrePath(grePath), fName(name), fTypes(types) {}

        const QString fGrePath;
        const QString fName;
        const QList<PropertyType> fTypes;
    };

    const QString fGrePath;
    const QList<stdsptr<PropertyCreator>> fProperties;
    GPURasterEffectProgram fProgram;

    bool compatible(const QList<PropertyType>& props) const {
        if(props.count() != fProperties.count()) return false;
        for(int i = 0; i < props.count(); i++) {
            const auto& iType = props.at(i);
            const auto prop = fProperties.at(i).get();
            if(iType == PTYPE_FLOAT) {
                const bool iCompatible =
                        dynamic_cast<QrealAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else if(iType == PTYPE_INT) {
                const bool iCompatible =
                        dynamic_cast<IntAnimatorCreator*>(prop);
                if(!iCompatible) return false;
            } else return false;
        }
        return true;
    }

    void reloadProgram(QGL33c * const gl, const QString& fragPath) {
        if(!QFile(fragPath).exists()) return;
        GPURasterEffectProgram program;
        try {
            program = GPURasterEffectProgram::sCreateProgram(
                        gl, fragPath, fProperties,
                        fProgram.fUniformCreators);
        } catch(...) {
            RuntimeThrow("Failed to load a new version of '" + fragPath + "'");
        }
        gl->glDeleteProgram(fProgram.fId);
        fProgram = program;
    }

    qsptr<Property> create() const;

    GLint getUniformLocation(QGL33c * const gl, const GLuint& program) const {
        Q_UNUSED(gl);
        Q_UNUSED(program);
        Q_ASSERT(false);
    }

    void writeIdentifier(QIODevice * const dst) const {
        gWrite(dst, fName);
        gWrite(dst, fGrePath);
        const int nChildren = fProperties.count();
        dst->write(rcConstChar(&nChildren), sizeof(int));
        for(const auto& anim : fProperties) {
            PropertyType type;
            if(dynamic_cast<QrealAnimatorCreator*>(anim.get())) {
                type = PTYPE_FLOAT;
            } else if(dynamic_cast<IntAnimatorCreator*>(anim.get())) {
                type = PTYPE_INT;
            } else RuntimeThrow("Only QrealAnimator and IntAnimator supported");
            dst->write(rcConstChar(&type), sizeof(PropertyType));
        }
    }

    static Identifier sReadIdentifier(QIODevice * const src) {
        const QString name = gReadString(src);
        const QString grePath = gReadString(src);
        QList<PropertyType> props;
        int nChildren;
        src->read(rcChar(&nChildren), sizeof(int));
        for(int i = 0; i < nChildren; i++) {
            PropertyType type;
            src->read(rcChar(&type), sizeof(PropertyType));
            props << type;
        }
        return Identifier(grePath, name, props);
    }

    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl, const QString& grePath);

    static stdsptr<GPURasterEffectCreator> sWithGrePath(
            const QString& grePath);

    static stdsptr<GPURasterEffectCreator> sWithGrePathAndCompatible(
            const QString& grePath,
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sWithName(
            const QString &name);

    static QList<stdsptr<GPURasterEffectCreator>> sWithNameAndCompatible(
            const QString &name,
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sWithCompatibleProps(
            const QList<PropertyType>& props);

    static QList<stdsptr<GPURasterEffectCreator>> sGetBestCompatibleEffects(
            const Identifier& id);

    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
};

#endif // GPURASTEREFFECTCREATOR_H
