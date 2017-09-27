#include "intanimator.h"

IntAnimator::IntAnimator() : QrealAnimator() {
    setNumberDecimals(0);
}

int IntAnimator::getCurrentIntValue() const {
    return qRound(qra_getCurrentValue());
}

int IntAnimator::getCurrentIntValueAtRelFrame(const int &relFrame) const {
    return qRound(getCurrentValueAtRelFrame(relFrame));
}

void IntAnimator::setCurrentIntValue(const int &value,
                                     const bool &saveUndoRedo,
                                     const bool &finish) {
    qra_setCurrentValue(value, saveUndoRedo, finish);
}

void IntAnimator::setIntValueRange(const int &minVal,
                                   const int &maxVal) {
    qra_setValueRange(minVal, maxVal);
}
