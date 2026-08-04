#ifndef MODEL_H
#define MODEL_H

#include <QtCore/qglobal.h>

namespace node {

struct Delta final {
    double initial {};
    double final {};

    double count {};
    double measure {};
    double discount {};

    bool IsNull() const { return qFuzzyIsNull(initial) && qFuzzyIsNull(final) && qFuzzyIsNull(count) && qFuzzyIsNull(measure) && qFuzzyIsNull(discount); }
};

}

#endif // MODEL_H
