/*
app_touch_pads.c
*/

#include "freertos/FreeRTOS.h"
#include "driver/touch_pad.h"
#include "soc/clk_tree_defs.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "app_timer.h"
#include "app_touch_pads.h"


#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (1)
#define MEASUREMENT_DURATION_MSEC  (4)
#define MEASUREMENT_INTERVAL_MSEC  (100 - MEASUREMENT_DURATION_MSEC)
#define FILTER_TOUCH_PERIOD_MSEC   (1000)

//TODO: implement this in "menuconfig".
// The difference between a prior value and a new value that triggers
//  the new value to be posted a an event.
#define UPDATE_THRESHOLD_VALUE 16

static const char* LOG_TAG = "app_touch_pads";

static uint16_t prior_touch_value[TOUCH_PAD_MAX];
static bool force_update = true;



/***
static void app_timer_tick_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    int64_t time_since_boot = esp_timer_get_time();
    ESP_LOGI(LOG_TAG, "App Touch Pad timer tick handler called: %lld us", time_since_boot);

    uint16_t touch_value;
#if TOUCH_FILTER_MODE_EN
    uint16_t touch_filter_value;
#endif

    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
#if TOUCH_FILTER_MODE_EN
        // If open the filter mode, please use this API to get the touch pad count.
        touch_pad_read_raw_data(i, &touch_value);
        touch_pad_read_filtered(i, &touch_filter_value);
#else
        touch_pad_read(i, &touch_value);
#endif
    }
}
***/



#if TOUCH_FILTER_MODE_EN
static void touch_filter_callback(uint16_t *raw_value, uint16_t *filtered_value)
{
    ESP_LOGV(LOG_TAG, "touch_filter_callback");

    uint16_t prior_value, new_value, diff;
    for (int ndx = 0; ndx < TOUCH_PAD_MAX; ++ndx) {
        prior_value = prior_touch_value[ndx];
        new_value = filtered_value[ndx];
        diff = prior_value > new_value ? prior_value - new_value : new_value - prior_value;
        if (force_update || diff > UPDATE_THRESHOLD_VALUE) {
            prior_touch_value[ndx] = new_value;
            ESP_LOGD(LOG_TAG, "touch - [%d] %u (diff=%u)", ndx, new_value, diff);
            //TODO: post this event.
        }
    }
    force_update = false;
}
#endif



void app_read_touch_pads_init(esp_event_loop_handle_t event_loop_handle)
{
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    ESP_ERROR_CHECK(touch_pad_init());

    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    //esp_err_t touch_pad_get_voltage(touch_high_volt_t *refh, touch_low_volt_t *refl, touch_volt_atten_t *atten)
    //esp_err_t touch_pad_set_voltage(touch_high_volt_t refh, touch_low_volt_t refl, touch_volt_atten_t atten)
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }

    // The following must be done last.
#if TOUCH_FILTER_MODE_EN
    /***
    {
        // The clock cycles of each measurement.
        //esp_err_t touch_pad_get_measurement_clock_cycles(uint16_t *clock_cycle)
        //esp_err_t touch_pad_set_measurement_clock_cycles(uint16_t clock_cycle)

        // The sleep cycles between two mesurements.
        //esp_err_t touch_pad_get_measurement_interval(uint16_t *interval_cycle)
        //esp_err_t touch_pad_set_measurement_interval(uint16_t interval_cycle)

        // Touch pad filter calibration period, in ms.
        //esp_err_t touch_pad_get_filter_period(uint32_t *p_period_ms)
        //esp_err_t touch_pad_set_filter_period(uint32_t new_period_ms)

        // Touch sensor charge/discharge speed for each pad.
        //esp_err_t touch_pad_get_cnt_mode(touch_pad_t touch_num, touch_cnt_slope_t *slope, touch_tie_opt_t *opt)
        //esp_err_t touch_pad_set_cnt_mode(touch_pad_t touch_num, touch_cnt_slope_t slope, touch_tie_opt_t opt)

        esp_err_t err;
        uint16_t sleep_cycle, meas_cycle;
        uint32_t filter_period_ms;
        err = touch_pad_get_measurement_clock_cycles(&meas_cycle);
        err = touch_pad_get_measurement_interval(&sleep_cycle);
        err = touch_pad_get_filter_period(&filter_period_ms);

        // e.g. meas_cycle=32767, sleep_cycle=4096, filter_period_ms=0 ms
        ESP_LOGI(LOG_TAG, "meas_cycle=%d, sleep_cycle=%d, filter_period_ms=%ld ms",
                 meas_cycle, sleep_cycle, filter_period_ms
        );
    }
    ***/

    // where:
    //   SOC_CLK_RC_FAST_FREQ_APPROX = 8,500,000
    //   SOC_CLK_RC_SLOW_FREQ_APPROX =   150,000
    //
    // measure_time = clock_cycle / SOC_CLK_RC_FAST_FREQ_APPROX
    // sleep_time = interval_cycle / SOC_CLK_RC_SLOW_FREQ_APPROX
    //
    // system default clock cycles:
    //   MEASUREMENT_CYCLES     32767
    //   MEASUREMENT_INTERVAL    4096

    uint16_t measurement_clock_cycles = MEASUREMENT_DURATION_MSEC * (SOC_CLK_RC_FAST_FREQ_APPROX / 1000),
             measurement_sleep_cycles = MEASUREMENT_INTERVAL_MSEC * (SOC_CLK_RC_SLOW_FREQ_APPROX / 1000);

    ESP_ERROR_CHECK(touch_pad_set_measurement_clock_cycles(measurement_clock_cycles));
    ESP_ERROR_CHECK(touch_pad_set_measurement_interval(measurement_sleep_cycles));

    //esp_err_t touch_pad_filter_start(uint32_t filter_period_ms)
    // filter calibration period, in ms
    ESP_ERROR_CHECK(touch_pad_filter_start(FILTER_TOUCH_PERIOD_MSEC));
    ESP_ERROR_CHECK(touch_pad_set_filter_read_cb(&touch_filter_callback));
#else
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
            event_loop_handle,
            APP_TIMER_EVENTS, APP_TIMER_TICK_EVENT,
            app_timer_tick_handler,
            event_loop_handle,
            NULL
    ));
#endif
}
