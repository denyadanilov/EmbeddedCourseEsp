#include "melody.h"
#include "player.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

melody_handle_t melody_handle = NULL;
player_handle_t melody_player_handle = NULL;

void app_main() {
  operation_result_t result = get_melody_by_id(MELODY_GAMA, &melody_handle); 

  if(result){
    return;
  }

  result = initialize_player(&melody_player_handle);

  if(result){
    return;
  }
  
  play_melody(melody_player_handle, melody_handle);

  while(is_melody_playing(melody_player_handle, melody_handle)){
    vTaskDelay(1);
  }

  release_melody(melody_handle);

  while(1){
    vTaskDelay(1);
  }
}