
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

extern long strtol(const char *nptr, char **endptr, int base);
long strtol(const char *nptr, char **endptr, int base) {
    const char *s;
    long acc, cutoff;
    int c;
    int neg, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    s = nptr;
    do {
        c = (unsigned char) *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else {
        neg = 0;
        if (c == '+')
            c = *s++;
    }
    if ((base == 0 || base == 16) &&
            c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? LONG_MIN : LONG_MAX;
    cutlim = cutoff % base;
    cutoff /= base;
    if (neg) {
        if (cutlim > 0) {
            cutlim -= base;
            cutoff += 1;
        }
        cutlim = -cutlim;
    }
    for (acc = 0, any = 0;; c = (unsigned char) *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0)
            continue;
        if (neg) {
            if (acc < cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LONG_MIN;
                errno = ERANGE;
            } else {
                any = 1;
                acc *= base;
                acc -= c;
            }
        } else {
            if (acc > cutoff || (acc == cutoff && c > cutlim)) {
                any = -1;
                acc = LONG_MAX;
                errno = ERANGE;
            } else {
                any = 1;
                acc *= base;
                acc += c;
            }
        }
    }
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (acc);
}

void mc_dc_case1(void)
{
    const char *test_string = "12345";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, 12345);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

void mc_dc_case2(void)
{
    const char *test_string = "-5432";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, -5432);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

void mc_dc_case3(void)
{
const char *test_string = "0x123";
char *endptr;
long result = strtol(test_string, &endptr, 16);
CU_ASSERT_EQUAL(result, 0x123);
CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}


void mc_dc_case4(void)
{
const char *test_string = "0x1G";
char *endptr;
long result = strtol(test_string, &endptr, 0);
CU_ASSERT_EQUAL(result, 1);
CU_ASSERT_PTR_EQUAL(endptr, test_string + 3);  // 停在 'G' 处
}

void mc_dc_case5(void)
{
const char *test_string = "9223372036854775808";  // 超过 LONG_MAX
char *endptr;
long result = strtol(test_string, &endptr, 10);
CU_ASSERT_EQUAL(result, LONG_MAX);
CU_ASSERT_PTR_EQUAL(endptr, test_string + 19);  // 停在开头
}

void mc_dc_case6(void)
{
const char *test_string = "-9223372036854775809";  // 超过 LONG_MIN
char *endptr;
long result = strtol(test_string, &endptr, 10);
CU_ASSERT_EQUAL(result, LONG_MIN);
CU_ASSERT_PTR_EQUAL(endptr, test_string + 20);  // 停在开头
}


void test_strtol() {
    const char *test_string = "12345";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);

    CU_ASSERT_EQUAL(result, 12345);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

int main() {
    // 初始化 CUnit 测试框架
    CU_initialize_registry();

    // 创建测试套件
    CU_pSuite suite = CU_add_suite("strtol_suite", NULL, NULL);
    
    // 添加测试用例到套件中
    CU_add_test(suite, "test_strtol", test_strtol);
    CU_add_test(suite, "mc_dc_case1", mc_dc_case1);
    CU_add_test(suite, "mc_dc_case2", mc_dc_case2);
    CU_add_test(suite, "mc_dc_case3", mc_dc_case3);
    CU_add_test(suite, "mc_dc_case4", mc_dc_case4);
    CU_add_test(suite, "mc_dc_case5", mc_dc_case5);
    CU_add_test(suite, "mc_dc_case6", mc_dc_case6);

    // 运行测试
    CU_basic_run_tests();

    // 清理 CUnit 测试框架资源
    CU_cleanup_registry();

    return 0;
}
