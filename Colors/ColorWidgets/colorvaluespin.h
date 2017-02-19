#ifndef COLORVALUESPIN_H
#define COLORVALUESPIN_H

#include <QObject>
#include <QDoubleSpinBox>
#include <GL/gl.h>

class QrealAnimatorValueSlider;

class ColorValueSpin : public QObject
{
    Q_OBJECT
public:
    ColorValueSpin(int min_val_t, int max_val_t,
                   int val_t, QWidget *parent = 0);
    ~ColorValueSpin();
    QrealAnimatorValueSlider *getSpinBox();
signals:
    void valSet(GLfloat);
public slots:
    void setVal(GLfloat val_t);
private slots:
    void spinBoxValSet(double spin_box_val_t);
private:
    bool mEmit = true;
    bool mBlockValue = false;
    QrealAnimatorValueSlider *mSpinBox = NULL;
};

#endif // COLORVALUESPIN_H
