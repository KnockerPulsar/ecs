
bool Between(float less, float value, float more)
{
    return (less < value) && (value < more);
}
bool BetweenEq(float less, float value, float more)
{
    return (less <= value) && (value <= more);
}

void XCHG(float &A, float &B)
{
    float temp = A;
    A = B;
    B = temp;
}
