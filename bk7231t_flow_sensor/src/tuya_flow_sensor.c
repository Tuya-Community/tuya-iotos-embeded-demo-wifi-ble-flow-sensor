/* Includes ------------------------------------------------------------------*/
#include "uni_log.h"
#include "soc_timer.h"
#include "tuya_key.h"
#include "tuya_hal_thread.h"
#include "tuya_hal_system.h"
#include "uni_thread.h"
#include "BkDriverGpio.h"

/* Private includes ----------------------------------------------------------*/
#include "tuya_flow_sensor.h"
#include "tuya_dp_process.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* timer */
#define Timer_1S    1

#define FLOW_SENSOR_PORT TY_GPIOA_6

/* Private variables ---------------------------------------------------------*/
static UINT_T sensor_pulse = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
VOID_T get_water_flow_cb(VOID_T)
{
    sensor_pulse++;
}

VOID_T timer_cb(VOID_T)
{
    BkGpioFinalize(FLOW_SENSOR_PORT);
    PR_NOTICE("sensor_pulse:%d", sensor_pulse);
    flow_sensor_state.flow_value = (sensor_pulse * 1000) / 450;
    PR_NOTICE("flow_sensor_state.flow_value: %d", flow_sensor_state.flow_value);
    hw_report_all_dp_status();
    sensor_pulse = 0;
}

VOID_T flow_sensor_task(VOID_T)
{
    while (1) {
        BkGpioEnableIRQ(FLOW_SENSOR_PORT, IRQ_TRIGGER_FALLING_EDGE, get_water_flow_cb, NULL);
        opSocSWTimerStart(Timer_1S, 1 * 1000, timer_cb);
        tuya_hal_system_sleep(2000);

        BkGpioFinalize(FLOW_SENSOR_PORT);
        opSocSWTimerStop(Timer_1S);
    }
}

VOID_T flow_sensor_init(VOID_T)
{
    tuya_gpio_inout_set(FLOW_SENSOR_PORT, TRUE);

    tuya_hal_thread_create(NULL, "get value", 512 * 4, TRD_PRIO_2, flow_sensor_task, NULL);
}
