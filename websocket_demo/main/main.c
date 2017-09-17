#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/adc.h"

#include "websocket.h"

#define SSID "myssid"
#define PASS "mywifipass"

void onOpen ( void ){
    printf ( "websocket connected\r\n" );
}

void onMessage ( websocket_frame_t * frame ){
    printf ( "receieved data: %s, data length: %d, frame type: %02x\r\n", frame->data, frame->length, frame->opcode );    
}

void onClose ( void ){
    printf ( "websocket closed\r\n" );
}

void websocket_task1 ( void * p ){

    websocket_t * websocket = new_websocket( IP_ADDR_ANY, 1000 );
    websocket->onopen = onOpen;
    websocket->onmessage = onMessage;
    websocket->onclose = onClose;

    char adc[10];

    while ( 1 ) {
        sprintf( adc, "%d", adc1_get_raw( ADC1_CHANNEL_0 ) );
        websocket_send ( websocket, ( u8_t * )adc, strlen ( adc ) );
    }
}

void websocket_task2 ( void * p ){

    websocket_t * websocket = new_websocket( IP_ADDR_ANY, 1001 );
    websocket->onopen = onOpen;
    websocket->onmessage = onMessage;
    websocket->onclose = onClose;

    char adc[10];
    
    while ( 1 ) {
        sprintf( adc, "%d", adc1_get_raw( ADC1_CHANNEL_1 ) );
        websocket_send ( websocket, ( u8_t * )adc, strlen ( adc ) );
    }
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    adc1_config_width( ADC_WIDTH_9Bit );

    xTaskCreate ( websocket_task1, "websocket1", 2048, NULL, 0, NULL );
    xTaskCreate ( websocket_task2, "websocket2", 2048, NULL, 0, NULL );

    vTaskDelete ( xTaskGetCurrentTaskHandle() );
}

