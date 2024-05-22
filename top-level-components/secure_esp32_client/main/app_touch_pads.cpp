/*
app_touch_pads.cpp
*/

#include "freertos/FreeRTOS.h"
#include "driver/touch_pad.h"
#include "soc/clk_tree_defs.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "app_timer.h"
#include "app_touch_pads.h"
#include "fast_array_average.hpp"


// Touch num 0 is denoise channel, please use `touch_pad_denoise_enable` to set denoise function.
// Ignore touch pad 0, start from touch pad 1.
#define FIRST_TOUCH_PAD_INDEX 1


//------------------------------------------------------------------------------
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
   // ESP32 S2 and S3 touch pads.
#  define USE_APP_TIMER_HANDLER
#  define TOUCH_VALUE_32_BIT

#elif CONFIG_IDF_TARGET_ESP32
   // ESP32 touch pads.
#  define USE_TOUCH_FILTER_CALLBACK
#  define TOUCH_VALUE_16_BIT

#else
#  error The device you are targeting is currently unsupported.
#endif
//------------------------------------------------------------------------------


#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define MEASUREMENT_DURATION_MSEC  (4)
#define MEASUREMENT_INTERVAL_MSEC  (100 - MEASUREMENT_DURATION_MSEC)
#define FILTER_TOUCH_PERIOD_MSEC   (1000)

//TODO: implement this in "menuconfig".
// The difference between a prior value and a new value that triggers
//  the new value to be posted a an event.
#define UPDATE_THRESHOLD_VALUE 16

ESP_EVENT_DEFINE_BASE(APP_TOUCH_EVENTS);

static const char* LOG_TAG = "app_touch_pads";


/****
TODO: document...

TouchValue_t:

TouchValuesAverage_t:

****/
#if defined(TOUCH_VALUE_16_BIT)
  using TouchValue_t = uint16_t;
  using TouchValuesAverage_t = FastArrayAverage<TouchValue_t, uint32_t, TOUCH_PAD_MAX>;
#elif defined(TOUCH_VALUE_32_BIT)
  using TouchValue_t = uint32_t;
  using TouchValuesAverage_t = FastArrayAverage<TouchValue_t, uint64_t, TOUCH_PAD_MAX>;
#endif


static TouchValuesAverage_t touchValuesAverage(5); // 2^5 = 32 (average over 32 samples)
static TouchValue_t prior_touch_value[TOUCH_PAD_MAX];
static bool force_update = true;

static esp_event_loop_handle_t event_loop_handle = NULL;


// see: github/espressif/esp-idf/components/hal/include/hal/touch_sensor_types.h
static const touch_pad_t TOUCH_PAD[ TOUCH_PAD_MAX ] = {
    TOUCH_PAD_NUM0,
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM2,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM9,
#if SOC_TOUCH_SENSOR_NUM > 10
    TOUCH_PAD_NUM10,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM12,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM14
#endif
};



static void post_touch_values(TouchValuesAverage_t::ValueArrayType& touch_values)
{
    // It's important to grab the current time at the top of this function.
    time_t now = 0;
    time(&now);

    ESP_LOGV(LOG_TAG, "post_touch_values");

    // Just incase force_update is changed while we are processing below.
    const bool local_force_update = force_update;
    force_update = false;

    TouchValue_t prior_value, new_value, diff;

    for (uint8_t ndx = FIRST_TOUCH_PAD_INDEX; ndx < TOUCH_PAD_MAX; ++ndx) {
        prior_value = prior_touch_value[ndx];
        new_value = touch_values[ndx];
        diff = prior_value > new_value ? prior_value - new_value : new_value - prior_value;
        if (local_force_update || diff > UPDATE_THRESHOLD_VALUE) {
            prior_touch_value[ndx] = new_value;
            ESP_LOGD(LOG_TAG, "touch - [%d] %u (diff=%u)", ndx, new_value, diff);

            app_touch_value_change_event_payload payload = {};
            payload.utc_timestamp = now;
            payload.touch_pad_num = ndx;
            payload.touch_value = new_value;

            esp_err_t err = esp_event_post_to(
                    event_loop_handle,
                    APP_TOUCH_EVENTS, APP_TOUCH_VALUE_CHANGE_EVENT,
                    &payload, sizeof(payload),
                    MEASUREMENT_DURATION_MSEC / portTICK_PERIOD_MS
            );
            switch(err) {
            case ESP_OK:
                // All is well
                break;
            case ESP_ERR_TIMEOUT:
                // Ignore and try again next time.
                ESP_LOGD(LOG_TAG, "APP_TOUCH_VALUE_CHANGE_EVENT timed-out! Ignoring and trying again.");
                prior_touch_value[ndx] = 0;
                // ?? force_update = true; ??
                break;
            default:
                ESP_ERROR_CHECK(err);
                break;
            }
        }
    }
}



static void handle_touch_values(TouchValuesAverage_t::ValueArrayType& touch_values)
{
    ESP_LOGV(LOG_TAG, "handle_touch_values");
    touchValuesAverage.add_values(touch_values);

    if (touchValuesAverage.is_average_ready()) {
        TouchValuesAverage_t::ValueArrayType average_values;
        touchValuesAverage.get_average_values(average_values);
        post_touch_values(average_values);
    }
}



#ifdef USE_APP_TIMER_HANDLER
static void app_timer_tick_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    TouchValuesAverage_t::ValueArrayType touch_values;

    for (uint8_t ndx = FIRST_TOUCH_PAD_INDEX; ndx < TOUCH_PAD_MAX; ++ndx) {
#if defined(TOUCH_VALUE_16_BIT)
        uint16_t tmp_u16;
        touch_pad_read_filtered(ndx, &tmp_u16);
        touch_values[ndx] = tmp_u16;
#elif defined(TOUCH_VALUE_32_BIT)
        uint32_t tmp_u32;
        touch_pad_filter_read_smooth(TOUCH_PAD[ndx], &tmp_u32);
        touch_values[ndx] = tmp_u32;
#endif
    }

    handle_touch_values(touch_values);
}
#endif



#ifdef USE_TOUCH_FILTER_CALLBACK
static void touch_filter_callback(uint16_t *raw_values, uint16_t *filtered_values)
{
    TouchValuesAverage_t::ValueArrayType touch_values;
    for (uint8_t ndx = 0; ndx < TOUCH_PAD_MAX; ++ndx) {
        touch_values[ndx] = filtered_values[ndx];
    }
    handle_touch_values(touch_values);
}
#endif



//------------------------------------------------------------------------------
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
// Initialize ESP32 S2 and S3 touch pads.
static void read_touch_pads_init_device()
{
    // Change the default touch sensor settings as needed.
    // touch_pad_set_measurement_interval(TOUCH_PAD_SLEEP_CYCLE_DEFAULT);
    // touch_pad_set_charge_discharge_times(TOUCH_PAD_MEASURE_CYCLE_DEFAULT);
    // touch_pad_set_voltage(TOUCH_PAD_HIGH_VOLTAGE_THRESHOLD, TOUCH_PAD_LOW_VOLTAGE_THRESHOLD, TOUCH_PAD_ATTEN_VOLTAGE_THRESHOLD);
    // touch_pad_set_idle_channel_connect(TOUCH_PAD_IDLE_CH_CONNECT_DEFAULT);
    // for (int i = 0; i < TOUCH_PAD_MAX; i++) {
    //     touch_pad_set_cnt_mode(button[i], TOUCH_PAD_SLOPE_DEFAULT, TOUCH_PAD_TIE_OPT_DEFAULT);
    // }

    /* Denoise setting at TouchSensor 0. */
    touch_pad_denoise_t denoise = {
        /* The bits to be cancelled are determined according to the noise level. */
        .grade = TOUCH_PAD_DENOISE_BIT4,
        .cap_level = TOUCH_PAD_DENOISE_CAP_L4,
    };
    touch_pad_denoise_set_config(&denoise);
    touch_pad_denoise_enable();

    /* Enable touch sensor clock. Work mode is "timer trigger". */
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_fsm_start();

    //TODO: write code to determine the better wait duration.
    // Wait 5 seconds for the touch pad filters to start doing their thing
    //  before we actually start listening for touch pad values.
    const TickType_t xdelay = 5000 / portTICK_PERIOD_MS;
    vTaskDelay(xdelay);

    // Now that the touch filters have had time to start doing their thing
    // start listening for the app timer events,
    // which regularly process the filtered touch values.
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(
            event_loop_handle,
            APP_TIMER_EVENTS, APP_TIMER_TICK_EVENT,
            app_timer_tick_handler,
            event_loop_handle,
            NULL
    ));
}


//------------------------------------------------------------------------------
#elif CONFIG_IDF_TARGET_ESP32
// Initialize ESP32 touch pads.
static void read_touch_pads_init_device()
{
    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    //esp_err_t touch_pad_get_voltage(touch_high_volt_t *refh, touch_low_volt_t *refl, touch_volt_atten_t *atten)
    //esp_err_t touch_pad_set_voltage(touch_high_volt_t refh, touch_low_volt_t refl, touch_volt_atten_t atten)
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

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
}
#endif



#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
   // For ESP32 S2 and S3, touch_pad_config() has only 1 argument.
   // This macro allows us to ignore the second argument.
#  define TOUCH_PAD_CONFIG(touch_num,threshold)  touch_pad_config(touch_num)

#elif CONFIG_IDF_TARGET_ESP32
   // For other targets, touch_pad_config() has 2 arguments.
#  define TOUCH_PAD_CONFIG(touch_num,threshold)  touch_pad_config(touch_num,threshold)

#endif


static void read_touch_pads_init_task(void *pvParameters)
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
    // touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    for (uint8_t ndx = 0; ndx < TOUCH_PAD_MAX; ++ndx) {
        TOUCH_PAD_CONFIG(TOUCH_PAD[ndx], TOUCH_THRESH_NO_USE);
    }

    // This must be done last.
    read_touch_pads_init_device();

    // This task has now finished doing what it needs to do.
    vTaskDelete(NULL);
}


void app_read_touch_pads_init(esp_event_loop_handle_t event_loop)
{
    event_loop_handle = event_loop;

    // Create a new task for initializing the touch pads so that we can
    // put in a delay to wait for the touch pad filters to start doing there thing.
    // TODO: usStackDepth should be better fine tuned.
    xTaskCreate(read_touch_pads_init_task, "read_touch_pads_init", 2048, NULL, uxTaskPriorityGet(NULL), NULL);
}
