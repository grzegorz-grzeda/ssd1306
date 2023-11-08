#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "cmocka.h"

#include "ssd1306.h"

typedef struct test_ssd1306 {
    generic_controller_t* ctrl;
    generic_display_t* display;
} test_ssd1306_t;

/* Mock controller */
static void mock_controller_reset_handler(const generic_controller_t* controller) {}

static void mock_controller_destroy_handler(const generic_controller_t* controller) {
    free((generic_controller_t*)controller);
}

static void mock_controller_send_command_handler(const generic_controller_t* controller,
                                                 const uint8_t* buffer,
                                                 size_t size) {}

static void mock_controller_send_data_handler(const generic_controller_t* controller,
                                              const uint8_t* buffer,
                                              size_t size) {}

static generic_controller_t* mock_controller_create(void) {
    generic_controller_t* ctrl = calloc(1, sizeof(*ctrl));
    ctrl->reset = mock_controller_reset_handler;
    ctrl->destroy = mock_controller_destroy_handler;
    ctrl->send_command = mock_controller_send_command_handler;
    ctrl->send_data = mock_controller_send_data_handler;
    return ctrl;
}
/* Mock controller expectations */
static void expect_mock_controller_reset(const generic_controller_t* controller) {
    expect_function_call(mock_controller_reset_handler);
    expect_value(mock_controller_reset_handler, controller, controller);
}

/* Unit tests */
static void test_try_to_create_with_invalid_controller(void** state) {
    generic_display_t* display = ssd1306_create(NULL);

    assert_ptr_equal(display, NULL);
}

static void test_reset_display(void** state) {}

/* Setup and teardown */
static int test_setup(void** state) {
    test_ssd1306_t* test_ssd1306 = calloc(1, sizeof(*test_ssd1306));
    test_ssd1306->ctrl = mock_controller_create();
    test_ssd1306->display = ssd1306_create(test_ssd1306->ctrl);
    *state = test_ssd1306;
    return 0;
}

static int test_teardown(void** state) {
    test_ssd1306_t* test_ssd1306 = (test_ssd1306_t*)*state;
    test_ssd1306->display->destroy(test_ssd1306->display);
    test_ssd1306->ctrl->destroy(test_ssd1306->ctrl);
    free(test_ssd1306);
    return 0;
}

int main(int argc, char** argv) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_try_to_create_with_invalid_controller),
        cmocka_unit_test_setup_teardown(test_reset_display, test_setup, test_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
