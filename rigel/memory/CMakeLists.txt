


rigel_cc_test(
        NAME
        object_pool_test
        SRCS
        "object_pool_test.cc"
        COPTS
        ${TURBO_TEST_COPTS}
        DEPS
        ${TURBO_LIBRARIES}
        rigel::rigel
        GTest::gmock_main
)