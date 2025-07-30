// app/src/humidity_sensor.c
#include "humidity_sensor.h"
#include "humidity_sensor_config.h"
#include "ecual_gpio.h"
#include "esp_log.h"
#include "dht.h"   // For DHT22
#include "sht3x.h" // For SHT3x

static const char *TAG = "HUMIDITY_SENSOR";
static dht_t dht_humidity_sensor_handles[HUMIDITY_TOTAL_SENSORS]; // Dedicated DHT handles for humidity module
static sht3x_t sht3x_sensor_handles[HUMIDITY_TOTAL_SENSORS]; // SHT3x handles

static const HUMIDITY_SENSOR_Config_t* humidity_sensor_get_config(HUMIDITY_SENSOR_ID_t sensor_id) {
    if (sensor_id >= HUMIDITY_TOTAL_SENSORS) {
        ESP_LOGE(TAG, "Invalid Humidity Sensor ID: %u", sensor_id);
        return NULL;
    }
    return &humidity_sensor_configurations[sensor_id];
}

uint8_t HUMIDITY_SENSOR_Init(void) {
    uint8_t app_ret = APP_OK;
    esp_err_t err;

    for (uint32_t i = 0; i < HUMIDITY_NUM_SENSOR_CONFIGURATIONS; i++) {
        const HUMIDITY_SENSOR_Config_t *cfg = &humidity_sensor_configurations[i];
        
        switch (cfg->sensor_type) {
            case HUMIDITY_TYPE_DHT22: {
                gpio_num_t gpio_num = (gpio_num_t)ECUAL_GPIO_GetPinNumber(cfg->gpio_pin_id);
                if (gpio_num == -1) {
                    ESP_LOGE(TAG, "DHT22 Humidity Sensor %u: Invalid ECUAL GPIO ID %u.", i, cfg->gpio_pin_id);
                    app_ret = APP_ERROR;
                    continue;
                }
                dht_init(&dht_humidity_sensor_handles[i], gpio_num, DHT_TYPE_DHT22);
                ESP_LOGI(TAG, "DHT22 Humidity Sensor %u initialized on GPIO %d.", i, gpio_num);
                break;
            }
            case HUMIDITY_TYPE_SHT3X: {
                i2c_port_t i2c_port = (i2c_port_t)cfg->params.sht3x.i2c_bus_id;
                uint8_t i2c_addr = cfg->params.sht3x.i2c_address;
                err = sht3x_init(&sht3x_sensor_handles[i], i2c_port, i2c_addr);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "SHT3X Humidity Sensor %u: Failed to initialize (err 0x%x).", i, err);
                    app_ret = APP_ERROR;
                    continue;
                }
                ESP_LOGI(TAG, "SHT3X Humidity Sensor %u initialized on I2C bus %d, address 0x%02X.",
                         i, i2c_port, i2c_addr);
                break;
            }
            default:
                ESP_LOGE(TAG, "Humidity Sensor %u: Unknown sensor type %u in config.", i, cfg->sensor_type);
                app_ret = APP_ERROR;
                break;
        }
    }
    return app_ret;
}

uint8_t HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_ID_t sensor_id, float *humidity_percent) {
    const HUMIDITY_SENSOR_Config_t *cfg = humidity_sensor_get_config(sensor_id);
    if (cfg == NULL || humidity_percent == NULL) { *humidity_percent = NAN; return APP_ERROR; }

    uint8_t app_ret = APP_OK;
    esp_err_t err;
    float temp_dummy; // Used for DHT/SHT3x, as they read both

    switch (cfg->sensor_type) {
        case HUMIDITY_TYPE_DHT22:
            err = dht_read_data(dht_humidity_sensor_handles[sensor_id], &temp_dummy, humidity_percent);
            if (err != ESP_OK) { app_ret = APP_ERROR; *humidity_percent = NAN; }
            break;
        case HUMIDITY_TYPE_SHT3X:
            err = sht3x_read(&sht3x_sensor_handles[sensor_id], &temp_dummy, humidity_percent);
            if (err != ESP_OK) { app_ret = APP_ERROR; *humidity_percent = NAN; }
            break;
        default: app_ret = APP_ERROR; *humidity_percent = NAN; break;
    }

    if (app_ret != APP_OK) { ESP_LOGE(TAG, "Humidity Sensor %u: Read failed.", sensor_id); }
    else { ESP_LOGI(TAG, "Humidity Sensor %u: %.2f %%", sensor_id, *humidity_percent); }
    return app_ret;
}
