// you can compile this code as "gcc inverseNum.c -o inverseNum -lgmp"
// this code computes inserse number for c in mod p

#include <stdio.h>
#include <gmp.h>

int main() {
    mpz_t c, p, inverse, mul;
    mpz_init(c);
    mpz_init(p);
    mpz_init(inverse);
    mpz_init(mul);

    mpz_set_ui(c, 28222);
    mpz_set_str(p, "2305843009213693951", 10);

    // Compute the modular inverse of c modulo p
    int result = mpz_invert(inverse, c, p);
    if (result == 0) {
        printf("Inverse does not exist\n");
    } else {
        // Verify that (c * inverse) mod p = 1
        mpz_t res;
        mpz_init(res);
        mpz_mul(res, c, inverse);
        mpz_mod(res, res, p);
        if (mpz_cmp_ui(res, 1) == 0) {
            mpz_mul(mul, inverse, c);
            gmp_printf("Inverse modulo %Zd of %Zd is %Zd\n", p, c, inverse);
            mpz_mod(mul, mul, p);
            gmp_printf("%Zd * %Zd is %Zd\n", c, inverse, mul);
        } else {
            printf("Inverse computation error\n");
        }
        mpz_clear(res);
    }   
    


    mpz_clear(c);
    mpz_clear(p);
    mpz_clear(inverse);
    mpz_clear(mul);

    return 0;
}