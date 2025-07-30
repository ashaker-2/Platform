// app/src/temp_sensor.c

#include "temp_sensor.h"
#include "temp_sensor_config.h"
#include "ecual_gpio.h" // For getting GPIO number from ECUAL_GPIO_ID
#include "ecual_adc.h"  // For ADC readings
#include "esp_log.h"

// Include specific sensor drivers
#include "ds18b20.h" // From espressif/ds18b20 component
#include "dht.h"     // From espressif/dht component

static const char *TAG = "TEMP_SENSOR";

// Array to store handles/references to initialized sensor drivers (if applicable)
static ds18b20_dev_t ds18b20_devices[TEMP_TOTAL_SENSORS]; // DS18B20 can have multiple on one bus
static dht_t dht_devices[TEMP_TOTAL_SENSORS]; // DHT is one per pin

// Helper to get sensor config by ID
static const TEMP_SENSOR_Config_t* temp_sensor_get_config(TEMP_SENSOR_ID_t sensor_id) {
    if (sensor_id >= TEMP_TOTAL_SENSORS) {
        ESP_LOGE(TAG, "Invalid Sensor ID: %u", sensor_id);
        return NULL;
    }
    return &temp_sensor_configurations[sensor_id];
}

uint8_t TEMP_SENSOR_Init(void) {
    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    for (uint32_t i = 0; i < TEMP_NUM_SENSOR_CONFIGURATIONS; i++) {
        const TEMP_SENSOR_Config_t *cfg = &temp_sensor_configurations[i];
        gpio_num_t gpio_num = (gpio_num_t)ECUAL_GPIO_GetPinNumber(cfg->gpio_pin_id); // Get physical GPIO num

        if (gpio_num == -1) {
            ESP_LOGE(TAG, "Sensor %u: Invalid ECUAL GPIO ID %u.", i, cfg->gpio_pin_id);
            app_ret = APP_ERROR;
            continue;
        }

        switch (cfg->sensor_type) {
            case TEMP_TYPE_DS18B20:
                // Initialize DS18B20 driver for this GPIO.
                // The ds18b20 component typically supports multiple sensors on one bus,
                // and finds them automatically. Here, we assume one DS18B20 per configured GPIO.
                ds18b20_init(gpio_num);
                ESP_LOGI(TAG, "DS18B20 Sensor %u initialized on GPIO %d.", i, gpio_num);
                break;

            case TEMP_TYPE_DHT22:
                // Initialize DHT driver
                dht_init(&dht_devices[i], gpio_num, DHT_TYPE_DHT22); // Use DHT_TYPE_DHT22 for DHT22
                ESP_LOGI(TAG, "DHT22 Sensor %u initialized on GPIO %d.", i, gpio_num);
                break;

            case TEMP_TYPE_LM35_ANALOG:
                // LM35 uses an ADC channel. ECUAL_ADC_Init should handle this.
                // We just ensure the ADC channel ID is valid.
                if (cfg->params.lm35.adc_channel_id >= ECUAL_ADC_TOTAL_CHANNELS) {
                    ESP_LOGE(TAG, "LM35 Sensor %u: Invalid ADC Channel ID %u in config.", i, cfg->params.lm35.adc_channel_id);
                    app_ret = APP_ERROR;
                    continue;
                }
                ESP_LOGI(TAG, "LM35 Sensor %u (ADC Channel %u, GPIO %d) configured.",
                         i, cfg->params.lm35.adc_channel_id, gpio_num);
                break;

            default:
                ESP_LOGE(TAG, "Sensor %u: Unknown sensor type %u in config.", i, cfg->sensor_type);
                app_ret = APP_ERROR;
                break;
        }
    }
    return app_ret;
}

uint8_t TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_ID_t sensor_id, float *temperature_celsius) {
    const TEMP_SENSOR_Config_t *cfg = temp_sensor_get_config(sensor_id);
    if (cfg == NULL || temperature_celsius == NULL) {
        if (temperature_celsius != NULL) *temperature_celsius = NAN;
        return APP_ERROR;
    }

    uint8_t app_ret = APP_OK;
    esp_err_t err = ESP_OK; // For underlying driver errors

    switch (cfg->sensor_type) {
        case TEMP_TYPE_DS18B20: {
            // DS18B20 driver usually finds devices and reads them
            float temp = ds18b20_get_temp(); // Gets temp from the *first* DS18B20 found on init GPIO
            if (temp == DS18B20_INVALID_TEMP) {
                 ESP_LOGE(TAG, "DS18B20 Sensor %u: Failed to read temperature.", sensor_id);
                 app_ret = APP_ERROR;
                 *temperature_celsius = NAN;
            } else {
                 *temperature_celsius = temp;
                 ESP_LOGI(TAG, "DS18B20 Sensor %u: Temp = %.2f C", sensor_id, *temperature_celsius);
            }
            break;
        }

        case TEMP_TYPE_DHT22: {
            float temp, humidity;
            err = dht_read_data(dht_devices[sensor_id], &temp, &humidity); // Needs the dht_t handle
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "DHT22 Sensor %u: Failed to read data (err 0x%x).", sensor_id, err);
                app_ret = APP_ERROR;
                *temperature_celsius = NAN;
            } else {
                *temperature_celsius = temp;
                ESP_LOGI(TAG, "DHT22 Sensor %u: Temp = %.2f C, Humidity = %.2f %%", sensor_id, temp, humidity);
            }
            break;
        }

        case TEMP_TYPE_LM35_ANALOG: {
            uint16_t adc_raw_value;
            uint8_t ecual_ret = ECUAL_ADC_ReadChannel(cfg->params.lm35.adc_channel_id, &adc_raw_value);
            if (ecual_ret != ECUAL_OK) {
                ESP_LOGE(TAG, "LM35 Sensor %u: Failed to read ADC channel %u.", sensor_id, cfg->params.lm35.adc_channel_id);
                app_ret = APP_ERROR;
                *temperature_celsius = NAN;
            } else {
                // LM35 formula: Temperature (C) = Voltage (mV) / 10
                // Voltage (mV) = (ADC_Raw_Value / ADC_Max_Value) * V_Ref_mV
                float voltage_mv = (float)adc_raw_value / ECUAL_ADC_MAX_VALUE * cfg->params.lm35.voltage_reference;
                *temperature_celsius = voltage_mv / 10.0f;
                ESP_LOGI(TAG, "LM35 Sensor %u: Raw ADC = %u, Voltage = %.2f mV, Temp = %.2f C",
                         sensor_id, adc_raw_value, voltage_mv, *temperature_celsius);
            }
            break;
        }

        default:
            ESP_LOGE(TAG, "Sensor %u: Unknown sensor type %u for ReadTemperature.", sensor_id, cfg->sensor_type);
            app_ret = APP_ERROR;
            *temperature_celsius = NAN;
            break;
    }

    return app_ret;
}

uint8_t TEMP_SENSOR_ReadHumidity(TEMP_SENSOR_ID_t sensor_id, float *humidity_percent) {
    const TEMP_SENSOR_Config_t *cfg = temp_sensor_get_config(sensor_id);
    if (cfg == NULL || humidity_percent == NULL) {
        if (humidity_percent != NULL) *humidity_percent = NAN;
        return APP_ERROR;
    }

    uint8_t app_ret = APP_OK;
    esp_err_t err = ESP_OK;

    if (cfg->sensor_type == TEMP_TYPE_DHT22) {
        float temp; // Temp also read but discarded if not needed
        err = dht_read_data(dht_devices[sensor_id], &temp, humidity_percent); // Needs the dht_t handle
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "DHT22 Sensor %u: Failed to read humidity (err 0x%x).", sensor_id, err);
            app_ret = APP_ERROR;
            *humidity_percent = NAN;
        } else {
            ESP_LOGI(TAG, "DHT22 Sensor %u: Humidity = %.2f %%", sensor_id, *humidity_percent);
        }
    } else {
        ESP_LOGW(TAG, "Sensor %u (Type %u) does not support humidity readings.", sensor_id, cfg->sensor_type);
        app_ret = APP_ERROR;
        *humidity_percent = NAN;
    }

    return app_ret;
}
