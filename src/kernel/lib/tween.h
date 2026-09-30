#pragma once

typedef enum
{
  TWEEN_LINEAR,
  TWEEN_EASE_IN_QUAD,
  TWEEN_EASE_OUT_QUAD,
  TWEEN_EASE_IN_OUT_QUAD,
  TWEEN_EASE_IN_CUBIC,
  TWEEN_EASE_OUT_CUBIC,
  TWEEN_EASE_IN_OUT_CUBIC,
  TWEEN_EASE_IN_QUART,
  TWEEN_EASE_OUT_QUART,
  TWEEN_EASE_IN_OUT_QUART,
  TWEEN_EASE_IN_QUINT,
  TWEEN_EASE_OUT_QUINT,
  TWEEN_EASE_IN_OUT_QUINT,
  TWEEN_EASE_IN_SINE,
  TWEEN_EASE_OUT_SINE,
  TWEEN_EASE_IN_OUT_SINE,
  TWEEN_EASE_IN_EXPO,
  TWEEN_EASE_OUT_EXPO,
  TWEEN_EASE_IN_OUT_EXPO
} tween_easing;

typedef struct
{
  float        From;
  float        To;
  float        Duration;
  float        Elapsed;
  float        Value;
  tween_easing Easing;
  int          Playing;
  int          Finished;
} tween;

void tween_init(tween *Tween, float From, float To, float Duration,
                tween_easing Easing);

void tween_start(tween *Tween);
void tween_stop(tween *Tween);
void tween_reset(tween *Tween);
void tween_update(tween *Tween, float DeltaTime);

float tween_get_value(tween *Tween);
float tween_get_progress(tween *Tween);

int tween_is_playing(tween *Tween);
int tween_is_finished(tween *Tween);
