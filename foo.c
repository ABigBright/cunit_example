#include <stdio.h>
#include <CUnit/Basic.h>

void foo(int a, int b) {
    if ((a > 0) && (b > 4)) {
        a += 4;
    }
}

void test_foo_both_conditions_true() {
    int a = 3;
    int b = 5;
    foo(a, b);
    CU_ASSERT_EQUAL(a, 3);
}

void test_foo_a_condition_false() {
    int a = -2;
    int b = 6;
    foo(a, b);
    CU_ASSERT_EQUAL(a, -2);
}

void test_foo_b_condition_false() {
    int a = 3;
    int b = 3;
    foo(a, b);
    CU_ASSERT_EQUAL(a, 3);
}

void test_foo_both_conditions_false() {
    int a = -2;
    int b = 3;
    foo(a, b);
    CU_ASSERT_EQUAL(a, -2);
}

void test_foo_a_and_b_conditions_true() {
    int a = 5;
    int b = 6;
    foo(a, b);
    CU_ASSERT_EQUAL(a, 5);
}

int main() {
    // 初始化 CUnit 测试框架
    CU_initialize_registry();

    // 创建测试套件
    CU_pSuite suite = CU_add_suite("foo_suite", NULL, NULL);

    // 添加测试用例到套件中
    CU_add_test(suite, "test_foo_both_conditions_true", test_foo_both_conditions_true);
    CU_add_test(suite, "test_foo_a_condition_false", test_foo_a_condition_false);
    CU_add_test(suite, "test_foo_b_condition_false", test_foo_b_condition_false);
    CU_add_test(suite, "test_foo_both_conditions_false", test_foo_both_conditions_false);
    CU_add_test(suite, "test_foo_a_and_b_conditions_true", test_foo_a_and_b_conditions_true);

    // 运行测试
    CU_basic_run_tests();

    // 清理 CUnit 测试框架资源
    CU_cleanup_registry();

    return 0;
}
