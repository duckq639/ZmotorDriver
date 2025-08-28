#ifndef MATHFUNC_H
#define MATHFUNC_H
#endif // !MATHFUNC_H

#define PEAK(x, max)           \
    do                         \
    {                          \
        if ((x) > (max))       \
            (x) = (max);       \
        else if ((x) < -(max)) \
            (x) = -(max);      \
    } while (0)

#define ABS(x) ((x) > 0 ? (x) : -(x))