#include "player.h"
#include "melody.h"
#include "errors.h"
#include "driver/ledc.h"
#include "driver/gptimer.h"
#include "esp_attr.h"

#define BUZZER_PIN 16
#define PERFORMANCE_DUTY ((1 << LEDC_TIMER_13_BIT) / 2)
#define SILENCE_DUTY 0
#define IDLE_FREQUENCY 5000
#define MICROSECONDS_IN_MILLISECOND 1000
#define ONE_MHZ 1000000

struct melody_context{
	melody_handle_t melody;
};

typedef struct melody_context melody_context_t;

int player_handle = 1;
ledc_timer_config_t ledc_timer_conf;
gptimer_handle_t gptimer;
gptimer_alarm_config_t gptimer_alarm_conf;
melody_context_t melody_ctx;

bool on_note_completed(struct gptimer_t *timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);
operation_result_t play_next_note(melody_handle_t melody);

operation_result_t initialize_player(player_handle_t *handle){

ledc_channel_config_t ledc_channel_conf = {
    .channel = LEDC_CHANNEL_0,
    .duty = SILENCE_DUTY,
    .gpio_num = BUZZER_PIN,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .hpoint = 0,
    .timer_sel = LEDC_TIMER_0
};

esp_err_t err = ledc_channel_config(&ledc_channel_conf);
if (err != ESP_OK) {
    return OPERATION_RESULT_ERROR;
}

ledc_timer_conf.duty_resolution = LEDC_TIMER_13_BIT;
ledc_timer_conf.freq_hz = IDLE_FREQUENCY;
ledc_timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
ledc_timer_conf.timer_num = LEDC_TIMER_0;
ledc_timer_conf.clk_cfg = LEDC_AUTO_CLK;

err = ledc_timer_config(&ledc_timer_conf);
if (err != ESP_OK) {
    return OPERATION_RESULT_ERROR;
}

gptimer_config_t gptimer_conf = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
	.direction = GPTIMER_COUNT_UP,
	.resolution_hz = ONE_MHZ
};

err = gptimer_new_timer(&gptimer_conf, &gptimer);

if (err != ESP_OK) {
    return OPERATION_RESULT_ERROR;
}

gptimer_event_callbacks_t gptimer_callbacks = {
    .on_alarm = on_note_completed
};
gptimer_register_event_callbacks(gptimer, &gptimer_callbacks, &melody_ctx);

*handle = &player_handle;

return OPERATION_RESULT_OK;
}

operation_result_t play_melody(player_handle_t player, melody_handle_t melody){
  if(!player || !melody){
    return OPERATION_RESULT_ERROR;
  }
  
  melody_ctx.melody = melody;
  operation_result_t result = play_next_note(melody);
  if(result != OPERATION_RESULT_OK){
    return OPERATION_RESULT_ERROR;
  }

  gptimer_enable(gptimer);
  gptimer_start(gptimer);

  return OPERATION_RESULT_OK;
}

bool is_melody_playing(player_handle_t player, melody_handle_t melody){
  if(!player || !melody){
    return false;
  }

  return melody_ctx.melody == melody;
}

IRAM_ATTR operation_result_t setup_to_play_next_note(uint32_t *frequency, uint32_t *duration){
    if (!frequency || !duration) {
        return OPERATION_RESULT_ERROR;
    }

	uint32_t target_frequency = *frequency == 0 ? IDLE_FREQUENCY : *frequency;
	uint32_t target_duty = *frequency == 0 ? SILENCE_DUTY : PERFORMANCE_DUTY;

	ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, target_frequency);
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, target_duty);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

	gptimer_alarm_conf.alarm_count = 0;
	gptimer_alarm_conf.flags.auto_reload_on_alarm = true;
	gptimer_alarm_conf.alarm_count = *duration * MICROSECONDS_IN_MILLISECOND;
	gptimer_set_alarm_action(gptimer, &gptimer_alarm_conf);

    return OPERATION_RESULT_OK;
}

IRAM_ATTR operation_result_t play_next_note(melody_handle_t melody){
uint32_t frequency, duration;
  operation_result_t result = get_next_note_params(melody, &frequency, &duration);

  if(result != OPERATION_RESULT_OK){
    return OPERATION_RESULT_ERROR;
  }

  result = setup_to_play_next_note(&frequency, &duration);

  if(result != OPERATION_RESULT_OK){
    return OPERATION_RESULT_ERROR;
  }
  return OPERATION_RESULT_OK;
}

IRAM_ATTR bool on_note_completed(struct gptimer_t *timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){

 melody_context_t* melody_ctx = (melody_context_t*)(user_ctx);
 operation_result_t result = play_next_note(melody_ctx->melody);
  if(result != OPERATION_RESULT_OK){
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, SILENCE_DUTY);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
	gptimer_stop(gptimer);
	gptimer_disable(gptimer);
  }

return true;
}