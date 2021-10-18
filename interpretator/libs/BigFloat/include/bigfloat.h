#ifndef BIG_FLOAT_H
#define BIG_FLOAT_H

#define BIGFLOAT_PRECISION 512

typedef struct bigfloat {
  unsigned char digits[BIGFLOAT_PRECISION];
  short decimal;
  unsigned char negative;
} BigFloat;

BigFloat *create(char *);
BigFloat *createFromInt(int);
void freeBigFloat(BigFloat *);
void parse(BigFloat *, char *);
char *toString(BigFloat *, unsigned char);
void add(BigFloat *, BigFloat *, BigFloat *);
void subtract(BigFloat *, BigFloat *, BigFloat *);
void multiply(BigFloat *, BigFloat *, BigFloat *);
char divide(BigFloat *, BigFloat *, BigFloat *);
char modulo(BigFloat *, BigFloat *, BigFloat *);
char equals(BigFloat *, BigFloat *);
char equalsUpTo(BigFloat *, BigFloat *, int);
char compare(BigFloat *, BigFloat *);
char compareDifference(BigFloat *a, BigFloat *b);
void clear(BigFloat *);
void standardizeDecimal(BigFloat *, BigFloat *);
void multiplyLine(BigFloat *, BigFloat *, int);
void zerosFirst(BigFloat *);
void intConvert(BigFloat *);
void trailingZeros(BigFloat *);
void shiftDownBy(char *, int, int);
void shiftUpBy(char *, int, int);
int toInt(BigFloat *);

#endif
