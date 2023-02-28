long goo(long arg9, long arg10) {
    long temp1 = 5 * arg9;
    long temp2 = arg10 / temp1;
    long temp3 = 5 * temp2;
    return temp3;
}

long boo(long arg6, long arg7, long arg8) {
    long temp1 = 5 * arg6;
    long temp2 = arg7 / arg8;
    long temp3 = temp1 / 2;
    long temp4 = goo(temp1, temp2);
    return temp4;
}

