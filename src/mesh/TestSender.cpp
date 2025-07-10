#include "TestSender.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "TestHdr.h"                   // struct test_hdr_t + TEST_ID
#include "MeshService.h"               // global  pointer  'service'
#include "NodeDB.h"
#include "RadioLibInterface.h"
#include "PowerStatus.h"
#include "Router.h"

static TimerHandle_t txTimer;
static uint32_t      seqCounter = 0;

static void timerCb(TimerHandle_t)
{
    // 1. Allocate a packet using Router
    auto *mp = router->allocForSending();
    if (!mp) return;

    mp->from      = nodeDB->getNodeNum();
    mp->to        = 0xffffffff;            // broadcast
    mp->id        = random();
    mp->hop_limit = 69;

    /* 2 ─ build a Data protobuf with our 20-byte header */
    meshtastic_Data data      = meshtastic_Data_init_default;
    data.portnum              = meshtastic_PortNum_TEST_BED;

    test_hdr_t hdr{};
    hdr.ver         = 1;
    hdr.test_id     = TEST_ID;
    hdr.seq         = ++seqCounter;
    hdr.src         = mp->from;
    hdr.tx_epoch_ms = millis();
    hdr.hop_cnt     = 0;
    hdr.batt_mV     = (powerStatus && powerStatus->getHasBattery())
                        ? powerStatus->getBatteryVoltageMv()
                        : 0;

    data.payload.size = sizeof(hdr);              // << correct nanopb field
    memcpy(data.payload.bytes, &hdr, sizeof(hdr));

    // 3. Set up the payload (protobuf, etc.)
    mp->decoded.portnum = data.portnum;
    mp->decoded.payload.size = data.payload.size;
    memcpy(mp->decoded.payload.bytes, data.payload.bytes, data.payload.size);
    mp->which_payload_variant = meshtastic_MeshPacket_decoded_tag;

    // 4. Send using Router
    ErrorCode res = router->sendLocal(mp, RX_SRC_USER);
    LOG_DEBUG("sent packet? %d", res);
}

namespace TestSender {
    void begin(uint32_t intervalMs)
    {
        txTimer = xTimerCreate("testTx",
                               pdMS_TO_TICKS(intervalMs),
                               pdTRUE,          // auto-reload
                               nullptr,
                               timerCb);
        xTimerStart(txTimer, 0);
    }
}
