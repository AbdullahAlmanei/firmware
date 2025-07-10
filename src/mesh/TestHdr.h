#pragma once
#include <stdint.h>
typedef struct __attribute__((packed)) {
    uint8_t  ver;
    uint32_t test_id;
    uint32_t seq;
    uint32_t src;
    uint32_t tx_epoch_ms;
    uint8_t  hop_cnt;
    uint16_t batt_mV;
} test_hdr_t;

#define TEST_ID 0xA5A5A5A5   // change per experiment
