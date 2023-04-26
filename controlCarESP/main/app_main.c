#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "driver/gpio.h"


#define CONFIG_BROKER_USER "pi"
#define CONFIG_BROKER_PASSWD "maciek"

#define GPIO_OUTPUT_PIN_LED 2
#define GPIO_OUTPUT_PIN_13 13
#define GPIO_OUTPUT_PIN_12 12
#define GPIO_OUTPUT_PIN_14 14
#define GPIO_OUTPUT_PIN_27 27

#define LEFT_PIN GPIO_OUTPUT_PIN_13
#define RIGHT_PIN GPIO_OUTPUT_PIN_12
#define FORWARD_PIN GPIO_OUTPUT_PIN_27
#define BACKWARD_PIN GPIO_OUTPUT_PIN_14

#define LEFT 0
#define RIGHT 1
#define FORWARD 2
#define BACKWARD 3

#define DIRECTIONS 4
#define HIGH 1
#define LOW 0

#define GPIO_OUTPUT_PIN_MASK ((1ULL<<GPIO_OUTPUT_PIN_LED) | (1ULL<<GPIO_OUTPUT_PIN_13) | (1ULL<<GPIO_OUTPUT_PIN_12) | (1ULL<<GPIO_OUTPUT_PIN_27) | (1ULL<<GPIO_OUTPUT_PIN_14))

static uint8_t direction_pins[DIRECTIONS] = {LEFT_PIN,RIGHT_PIN,FORWARD_PIN,BACKWARD_PIN}; //need only for initialization
static uint8_t direction_pins_state[DIRECTIONS] = {1,1,1,1};

static const char *TAG = "INFO MQTT";


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 *  This function is called by the MQTT client event loop.
 */

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "/esp32/controlCar", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        switch (*event->data)
        {
        case 'f':
            direction_pins_state[FORWARD]^=1U;
            gpio_set_level(FORWARD_PIN,direction_pins_state[FORWARD]);
            printf("FORWARD: %d\n", direction_pins_state[FORWARD]);
            break;
        case 'b':
            direction_pins_state[BACKWARD]^=1;
            gpio_set_level(BACKWARD_PIN,direction_pins_state[BACKWARD]);
            break;
            printf("BACKWARD\n");
        case 'l':
            direction_pins_state[LEFT]^=1;
            gpio_set_level(LEFT_PIN,direction_pins_state[LEFT]);
            printf("LEFT\n");
            break;
        case 'r':
            direction_pins_state[RIGHT]^=1;
            gpio_set_level(RIGHT_PIN,direction_pins_state[RIGHT]);
            printf("RIGHT\n");
            break;
        default:
            break;
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .credentials.username= CONFIG_BROKER_USER,
        .credentials.authentication.password=CONFIG_BROKER_PASSWD,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


static void configure_gpio(void){
    esp_rom_gpio_pad_select_gpio(GPIO_OUTPUT_PIN_LED);
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_MASK;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    for(uint8_t i=0; i<DIRECTIONS; i++){
        gpio_set_level(direction_pins[i],HIGH);
    }
}

void app_main(void)
{
    configure_gpio();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();   
}
