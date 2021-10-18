#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bigfloat.h"

/*
* Create BigFloat from char array, and return the pointer.
*/
BigFloat *create(char *str)
{
    int i;
    BigFloat *res;
    res = malloc(sizeof(BigFloat));
    res->decimal = 1;
    for (i = 0; i < BIGFLOAT_PRECISION; i++)
    {
        res->digits[i] = 0;
    }
    res->negative = 0;
    parse(res, str);
    return res;
}

/*
* Create BigFloat from integer value, and return the pointer.
*/
BigFloat *createFromInt(int value)
{
    int i;
    BigFloat *res;
    res = malloc(sizeof(BigFloat));
    res->decimal = 1;
    for (i = 0; i < BIGFLOAT_PRECISION; i++)
    {
        res->digits[i] = 0;
    }
    res->negative = (value < 0);
    int valueSize = (floor(log10(abs(value))) + 1);
    for (i = 0; i < valueSize; i++)
    {
        res->digits[i] = value % 10;
        value /= 10;
        res->decimal++;
    }
    return res;
}

void freeBigFloat(BigFloat *b)
{
    if (b != NULL)
        free(b);
}

/*
* Parses in a string representing a floating point number and creates a
* BigFloat out of the string representation.
*/
void parse(BigFloat *b, char *str)
{
    if (str == NULL)
    {
        clear(b);
        b->negative = 0;
        b->decimal = 1;
        return;
    }
    b->decimal = 0;
    int i = 0;
    int index = 0;
    if (str[0] == '-')
    {
        b->negative = 1;
        i = 1;
    }
    else
    {
        b->negative = 0;
    }
    for (; i < strlen(str) && index < BIGFLOAT_PRECISION; i++)
    {
        if (str[i] == '.')
        {
            b->decimal = (b->negative) ? i - 1 : i;
        }
        else
        {
            b->digits[index++] = str[i] - '0';
        }
    }
    if (b->decimal == 0)
        b->decimal = (strlen(str) - b->negative);
}

/*
* Converts BigFloat into a char array.
*/
char *toString(BigFloat *b, unsigned char decimals)
{
    const unsigned short length = (BIGFLOAT_PRECISION + b->negative + 1);
    char *str = malloc(sizeof(char) * (BIGFLOAT_PRECISION + 2));
    if (str == NULL)
        return NULL;
    memcpy((str + b->negative), b->digits, (BIGFLOAT_PRECISION + b->negative));
    if (b->negative)
        str[0] = '-';
    short i;
    if (decimals)
    {
        for (i = (BIGFLOAT_PRECISION + b->negative); i > b->decimal; i--)
        {
            if (str[(i - 1)] != 0)
                break;
        }
    }
    else
        i = b->decimal;
    for (short ii = b->negative; ii < i; ii++)
        str[ii] += '0';
    memset((str + i), 0, (length - i));
    str[i] = '\0';
    unsigned short new_length = (sizeof(char) * (i + (decimals != 0) + b->negative + 1));
    char *str_short = realloc(str, new_length);
    if (str_short != NULL)
    {
        str = str_short;
        str[(i + (decimals != 0) + b->negative)] = '\0';
    }
    if (decimals)
    {
        memmove((str + b->negative + b->decimal + 1), (str + b->negative + b->decimal), (new_length - b->decimal));
        str[(b->negative + b->decimal)] = '.';
    }
    return str;
}

/*
* Adds two BigFloats and puts the result in the first parameter.
*/
void add(BigFloat *a, BigFloat *b, BigFloat *res)
{
    int i, result;
    int carry = 0;
    standardizeDecimal(a, b);
    clear(res);
    res->decimal = a->decimal;
    unsigned char additionType = (a->negative + b->negative);
    if (additionType != 1)
    {
        for (i = BIGFLOAT_PRECISION - 1; i >= 0; i--)
        {
            result = carry;
            result += a->digits[i] + b->digits[i];
            carry = result / 10;
            res->digits[i] = result % 10;
        }
        if (carry != 0)
        {
            shiftDownBy((char *)res->digits, BIGFLOAT_PRECISION, 1);
            res->decimal++;
            res->digits[0] = carry;
        }
        trailingZeros(a);
        trailingZeros(b);
        trailingZeros(res);
        if (additionType == 2)
            res->negative = 1;
    }
    else if (a->negative)
    {
        b->negative = 1;
        subtract(b, a, res);
        b->negative = 0;
    }
    else if (b->negative)
    {
        a->negative = 1;
        subtract(a, b, res);
        a->negative = 0;
    }
}

/*
* Subtract b from a and return a new BigFloat as the result.
*/
void subtract(BigFloat *a, BigFloat *b, BigFloat *res)
{
    int i, result;
    int carry = 0;
    BigFloat *top, *bottom;
    standardizeDecimal(a, b);
    clear(res);
    res->decimal = a->decimal;
    if (compare(a, b) >= 0)
    {
        top = a;
        bottom = b;
    }
    else
    {
        top = b;
        bottom = a;
        res->negative = 1;
    }
    for (i = BIGFLOAT_PRECISION - 1; i >= 0; i--)
    {
        result = carry + top->digits[i];
        if (result < bottom->digits[i])
        {
            carry = -1;
            res->digits[i] = result + 10 - bottom->digits[i];
        }
        else
        {
            carry = 0;
            res->digits[i] = result - bottom->digits[i];
        }
    }
    trailingZeros(a);
    trailingZeros(b);
    trailingZeros(res);
}

void multiply(BigFloat *a, BigFloat *b, BigFloat *res)
{
    int i;
    BigFloat *line = create(NULL);
    BigFloat *temp = create(NULL);
    clear(res);
    res->decimal = BIGFLOAT_PRECISION;
    line->decimal = BIGFLOAT_PRECISION;
    zerosFirst(a);
    zerosFirst(b);
    for (i = BIGFLOAT_PRECISION - 1; i >= 0; i--)
    {
        multiplyLine(a, line, b->digits[i]);
        shiftUpBy((char *)line->digits, BIGFLOAT_PRECISION, BIGFLOAT_PRECISION - i);
        add(res, line, temp);
        line->decimal = BIGFLOAT_PRECISION;
        zerosFirst(temp);
        memcpy(res, temp, sizeof(BigFloat));
    }
    res->decimal -= BIGFLOAT_PRECISION - a->decimal + BIGFLOAT_PRECISION - b->decimal + 1;
    trailingZeros(a);
    trailingZeros(b);
    trailingZeros(res);
    freeBigFloat(line);
    line = NULL;
    res->negative = ((a->negative || b->negative) && !(a->negative && b->negative)) ? 1 : 0;
    freeBigFloat(temp);
}

void multiplyLine(BigFloat *a, BigFloat *line, int mult)
{
    int i, result;
    int carry = 0;
    for (i = BIGFLOAT_PRECISION - 1; i >= 0; i--)
    {
        result = carry;
        result += a->digits[i] * mult;
        carry = result / 10;
        line->digits[i] = result % 10;
    }
}

char divide(BigFloat *a, BigFloat *b, BigFloat *res)
{
    int i, counter;
    int index = 0;
    clear(res);
    res->decimal = b->decimal;
    if (equals(b, res))
        return 1;
    BigFloat *current = create(NULL);
    BigFloat *temp = create(NULL);
    current->decimal = 0;
    res->decimal = a->decimal;
    for (i = 0; i < BIGFLOAT_PRECISION; i++)
    {
        counter = 0;
        current->digits[index++] = a->digits[i];
        current->decimal++;
        trailingZeros(current);
        for (;compare(current, b) >= 0;)
        {
            subtract(current, b, temp);
            memcpy(current, temp, sizeof(BigFloat));
            counter++;
        }
        res->digits[i] = counter;
    }
    freeBigFloat(temp);
    freeBigFloat(current);
    trailingZeros(res);
    return 0;
}

char modulo(BigFloat *a, BigFloat *b, BigFloat *res)
{
    BigFloat *temp = create(NULL);
    BigFloat *temp2 = create(NULL);
    BigFloat *temp3 = create("0.5");
    clear(res);
    if (equals(b, res))
        return 1;
    divide(a, b, temp);
    memcpy(temp2, temp, sizeof(BigFloat));
    intConvert(temp);
    if (temp->negative)
    {
        // BigFloat *temp4 = create(NULL);
        // subtract(temp, temp3, temp4);
        // intConvert(temp4);
        // if (!equals(temp, temp4))
        //     memcpy(temp, temp4, sizeof(BigFloat));
        // freeBigFloat(temp4);
    }
    multiply(temp, b, temp2);
    subtract(a, temp2, res);
    freeBigFloat(temp);
    freeBigFloat(temp2);
    freeBigFloat(temp3);
    return 0;
}

/*
* Tests whether or not two BigFloats are equal.
*/
char equals(BigFloat *a, BigFloat *b)
{
    int i;
    if (a == b)
    {
        return 1;
    }
    else
    {
        if (a->decimal == b->decimal && a->negative == b->negative)
        {
            for (i = 0; i < BIGFLOAT_PRECISION; i++)
            {
                if (a->digits[i] != b->digits[i])
                {
                    return 0;
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

/*
* Tests whether or not two BigFloats are equal up to the given decimal place.
*/
char equalsUpTo(BigFloat *a, BigFloat *b, int decimal)
{
    int i;
    if (a == b)
    {
        return 1;
    }
    else
    {
        if (a->decimal == b->decimal && a->negative == b->negative)
        {
            for (i = 0; i < a->decimal + decimal; i++)
            {
                if (a->digits[i] != b->digits[i])
                {
                    return 0;
                }
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

/*
* Compares two BigFloats so that compare(a, b) > 0 if
* a > b and so on with = and <
*/
char compare(BigFloat *a, BigFloat *b)
{
    int i;
    if (a == b)
    {
        return 0;
    }
    else
    {
        if (a->decimal != b->decimal)
        {
            return (char)a->decimal - b->decimal;
        }
        else
        {
            for (i = 0; i < BIGFLOAT_PRECISION; i++)
            {
                if (a->digits[i] != b->digits[i])
                {
                    return (char)a->digits[i] - b->digits[i];
                }
            }
            return 0;
        }
    }
}

char compareDifference(BigFloat *a, BigFloat *b)
{
    standardizeDecimal(a, b);
    if (a->negative && !b->negative)
        return -1;
    else if (!a->negative && b->negative)
        return 1;
    else if (a->digits < b->digits)
        return !a->negative;
    else if (a->digits > b->digits)
        return !b->negative;
    else
        return 0;
}

/*
* Shifts the BigFloat down so that there are not any trailing zeros and all
* zeros are leading the BigFloat.
*/
void zerosFirst(BigFloat *a)
{
    int i, start;
    for (i = BIGFLOAT_PRECISION - 1; i >= 0 && !a->digits[i]; i--)
        ;
    start = i;
    shiftDownBy((char *)a->digits, BIGFLOAT_PRECISION, BIGFLOAT_PRECISION - start - 1);
    a->decimal += BIGFLOAT_PRECISION - start - 1;
}

/*
* Removes any digits after decimal position.
*/
void intConvert(BigFloat *a)
{
    int i;
    for (i = (BIGFLOAT_PRECISION - 1); i >= a->decimal; i--)
        a->digits[i] = 0;
}

/*
* Shifts the BigFloat down so that there are not any leading zeros and all
* zeros are trailing the BigFloat.
*/
void trailingZeros(BigFloat *a)
{
    int i, start;
    for (i = 0; i < BIGFLOAT_PRECISION && !a->digits[i]; i++)
        ;
    if (a->decimal - i < 1)
    {
        i = a->decimal - 1;
    }
    start = i;
    shiftUpBy((char *)a->digits, BIGFLOAT_PRECISION, start);
    a->decimal -= start;
}

/*
* Takes two BigFloats and shifts them so that they have the same decimal point.
*/
void standardizeDecimal(BigFloat *a, BigFloat *b)
{
    if (b->decimal > a->decimal)
    {
        shiftDownBy((char *)a->digits, BIGFLOAT_PRECISION, b->decimal - a->decimal);
        a->decimal = b->decimal;
    }
    else if (b->decimal < a->decimal)
    {
        shiftDownBy((char *)b->digits, BIGFLOAT_PRECISION, a->decimal - b->decimal);
        b->decimal = a->decimal;
    }
}

/*
* Shifts a char array down by the specified shift
*/
void shiftDownBy(char *ar, int length, int shift)
{
    int i;
    for (i = length - 1; i >= 0; i--)
    {
        if (i - shift >= 0)
        {
            ar[i] = ar[i - shift];
        }
        else
        {
            ar[i] = 0;
        }
    }
}

/*
* Shifts a char array up by the specified shift
*/
void shiftUpBy(char *ar, int length, int shift)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (i + shift < length)
        {
            ar[i] = ar[i + shift];
        }
        else
        {
            ar[i] = 0;
        }
    }
}

void clear(BigFloat *a)
{
    int i;
    if (a != NULL)
    {
        for (i = 0; i < BIGFLOAT_PRECISION; i++)
        {
            a->digits[i] = 0;
        }
    }
}

int toInt(BigFloat *a)
{
    int result = 0;
    for (unsigned char i = 0; (i < a->decimal && i < 10); i++)
    {
        result *= 10;
        result += a->digits[i];
    }
    if (a->negative)
        result = (0 - result);
    return result;
}
