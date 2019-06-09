#ifndef CUSTOMPATHEFFECT_H
#define CUSTOMPATHEFFECT_H
#include "patheffect.h"

class CustomPathEffect : public PathEffect {
protected:
    CustomPathEffect(const QString& name);
public:
    virtual QByteArray getIdentifier() const = 0;

    virtual void read(const QByteArray& identifier,
                      QIODevice * const dst) = 0;
    virtual void write(QIODevice * const dst) const = 0;

    void readProperty(QIODevice * const src) final;
    void writeProperty(QIODevice * const target) const final;
};

#endif // CUSTOMPATHEFFECT_H
