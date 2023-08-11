#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <ctype.h>
#include <limits.h>

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
void test_strtol() {
    const char *test_string = "12345";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, 12345);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

void test_change_sign() {
    const char *test_string = "-9876";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, -9876);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

void test_change_base() {
    const char *test_string = "0xABCD";
    char *endptr;
    long result = strtol(test_string, &endptr, 16);
    CU_ASSERT_EQUAL(result, 43981);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 6);
}

// ... 更多的改变条件值的测试用例

void test_change_result() {
    const char *test_string = "12345";
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_NOT_EQUAL(result, 9999);  // 改变期望的结果
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 5);
}

void test_change_cutlim() {
    const char *test_string = "0xABCD";
    char *endptr;
    long result = strtol(test_string, &endptr, 16);
    CU_ASSERT_EQUAL(result, 43981);
    CU_ASSERT_PTR_NOT_EQUAL(endptr, test_string + 6);  // 停在 'x' 或 'X'
}

void test_change_overflow() {
    const char *test_string = "9223372036854775808";  // 超过 LONG_MAX
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, LONG_MAX);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 19);
}

void test_change_negative_overflow() {
    const char *test_string = "-9223372036854775809";  // 超过 LONG_MIN
    char *endptr;
    long result = strtol(test_string, &endptr, 10);
    CU_ASSERT_EQUAL(result, LONG_MIN);
    CU_ASSERT_PTR_EQUAL(endptr, test_string + 20);
}

int main() {
    // 初始化 CUnit 测试框架
    CU_initialize_registry();

    // 创建测试套件
    CU_pSuite suite = CU_add_suite("strtol_suite", NULL, NULL);

    // 添加测试用例到套件中
    CU_add_test(suite, "test_strtol", test_strtol);
    CU_add_test(suite, "test_change_sign", test_change_sign);
    CU_add_test(suite, "test_change_base", test_change_base);
    CU_add_test(suite, "test_change_result", test_change_result);
    CU_add_test(suite, "test_change_cutlim", test_change_cutlim);
    CU_add_test(suite, "test_change_overflow", test_change_overflow);
    CU_add_test(suite, "test_change_negative_overflow", test_change_negative_overflow);

    // 运行测试
    CU_basic_run_tests();

    // 清理 CUnit 测试框架资源
    CU_cleanup_registry();

    return 0;
}
