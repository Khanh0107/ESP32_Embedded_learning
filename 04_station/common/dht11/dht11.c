#include "dht11.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static gpio_num_t dht_gpio;
static int64_t last_read_time = -2000000;   // 2s
static struct dht11_reading last_read;

/* Wait for pin to change level with timeout (us) */
static int _waitOrTimeout(uint32_t microSeconds, int level)
{
    uint32_t micros_ticks = 0;

    while (gpio_get_level(dht_gpio) == level) {
        if (micros_ticks++ > microSeconds) {
            return DHT11_TIMEOUT_ERROR;
        }
        ets_delay_us(1);
    }

    return micros_ticks;
}

/* Check CRC */
static int _checkCRC(uint8_t data[5])
{
    if (data[4] == (data[0] + data[1] + data[2] + data[3])) {
        return DHT11_OK;
    }
    return DHT11_CRC_ERROR;
}

/* Send start signal */
static void _sendStartSignal()
{
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    ets_delay_us(20 * 1000);      // 20ms LOW
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(40);             // 40us HIGH
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);
}

/* Init */
void DHT11_init(gpio_num_t gpio_num)
{
    dht_gpio = gpio_num;

    gpio_pad_select_gpio(dht_gpio);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
}

/* Read DHT11 */
struct dht11_reading DHT11_read()
{
    struct dht11_reading result;
    int64_t now = esp_timer_get_time();

    // DHT11 chỉ đọc mỗi 2 giây
    if ((now - last_read_time) < 2000000) {
        return last_read;
    }

    last_read_time = now;

    uint8_t data[5] = {0};

    _sendStartSignal();

    // Check phản hồi từ DHT
    if (_waitOrTimeout(80, 0) == DHT11_TIMEOUT_ERROR) {
        result.status = DHT11_TIMEOUT_ERROR;
        return result;
    }

    if (_waitOrTimeout(80, 1) == DHT11_TIMEOUT_ERROR) {
        result.status = DHT11_TIMEOUT_ERROR;
        return result;
    }

    // Đọc 40 bit
    for (int i = 0; i < 40; i++) {

        if (_waitOrTimeout(50, 0) == DHT11_TIMEOUT_ERROR) {
            result.status = DHT11_TIMEOUT_ERROR;
            return result;
        }

        int t = _waitOrTimeout(70, 1);
        if (t == DHT11_TIMEOUT_ERROR) {
            result.status = DHT11_TIMEOUT_ERROR;
            return result;
        }

        data[i / 8] <<= 1;

        if (t > 40) {  // bit 1
            data[i / 8] |= 1;
        }
    }

    result.status = _checkCRC(data);
    if (result.status != DHT11_OK) {
        return result;
    }

    // ============================
    // Xử lý số thập phân đúng chuẩn
    // ============================

    // Độ ẩm
    result.humidity = (float)data[0] + ((float)data[1] * 0.1f);

    // Nhiệt độ
    result.temperature = (float)data[2];

    // Kiểm tra bit dấu (hiếm khi có ở DHT11)
    if (data[3] & 0x80) {
        result.temperature = -result.temperature;
    }

    result.temperature += (float)(data[3] & 0x0F) * 0.1f;

    last_read = result;

    return result;
}