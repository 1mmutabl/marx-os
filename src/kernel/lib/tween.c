#include "tween.h"

static float tween_clamp(float Value, float Min, float Max)
{
  if (Value < Min)
    return Min;

  if (Value > Max)
    return Max;

  return Value;
}

static float tween_pow2(float Value)
{
  float Result = 1.f;

  if (Value < 0.f)
  {
    Value = -Value;

    while (Value >= 1.f)
    {
      Result *= 0.5f;
      Value -= 1.f;
    }

    return Result;
  }

  while (Value >= 1.f)
  {
    Result *= 2.f;
    Value -= 1.f;
  }

  return Result;
}

float tween_ease(float Value, tween_easing Easing)
{
  float X;

  Value = tween_clamp(Value, 0.f, 1.f);

  switch (Easing)
  {
    case TWEEN_LINEAR:
      return Value;

    case TWEEN_EASE_IN_QUAD:
      return Value * Value;

    case TWEEN_EASE_OUT_QUAD:
      X = 1.f - Value;
      return 1.f - X * X;

    case TWEEN_EASE_IN_OUT_QUAD:
      if (Value < 0.5f)
        return 2.f * Value * Value;

      X = 1.f - Value;
      return 1.f - 2.f * X * X;

    case TWEEN_EASE_IN_CUBIC:
      return Value * Value * Value;

    case TWEEN_EASE_OUT_CUBIC:
      X = 1.f - Value;
      return 1.f - X * X * X;

    case TWEEN_EASE_IN_OUT_CUBIC:
      if (Value < 0.5f)
        return 4.f * Value * Value * Value;

      X = 1.f - Value;
      return 1.f - 4.f * X * X * X;

    case TWEEN_EASE_IN_QUART:
      X = Value * Value;
      return X * X;

    case TWEEN_EASE_OUT_QUART:
      X = 1.f - Value;
      X = X * X;
      return 1.f - X * X;

    case TWEEN_EASE_IN_OUT_QUART:
      if (Value < 0.5f)
      {
        X = Value * Value;
        return 8.f * X * X;
      }

      X = 1.f - Value;
      X = X * X;

      return 1.f - 8.f * X * X;

    case TWEEN_EASE_IN_QUINT:
      X = Value * Value;
      return X * X * Value;

    case TWEEN_EASE_OUT_QUINT:
      X = 1.f - Value;
      return 1.f - X * X * X * X * X;

    case TWEEN_EASE_IN_OUT_QUINT:
      if (Value < 0.5f)
        return 16.f * Value * Value * Value * Value * Value;

      X = 1.f - Value;
      return 1.f - 16.f * X * X * X * X * X;

    case TWEEN_EASE_IN_SINE:
      X = Value * Value;
      return Value - X * 0.5f;

    case TWEEN_EASE_OUT_SINE:
      X = 1.f - Value;
      return 1.f - X * X * 0.5f;

    case TWEEN_EASE_IN_OUT_SINE:
      if (Value < 0.5f)
      {
        X = Value * 2.f;
        return (X - X * X * 0.5f) * 0.5f;
      }

      X = (1.f - Value) * 2.f;
      return 1.f - (X - X * X * 0.5f) * 0.5f;

    case TWEEN_EASE_IN_EXPO:
      if (Value == 0.f)
        return 0.f;

      return tween_pow2(Value * 10.f - 10.f);

    case TWEEN_EASE_OUT_EXPO:
      if (Value == 1.f)
        return 1.f;

      return 1.f - tween_pow2(-10.f * Value);

    case TWEEN_EASE_IN_OUT_EXPO:
      if (Value == 0.f)
        return 0.f;

      if (Value == 1.f)
        return 1.f;

      if (Value < 0.5f)
        return tween_pow2(Value * 20.f - 10.f) * 0.5f;

      return 1.f - tween_pow2(-Value * 20.f + 10.f) * 0.5f;
  }

  return Value;
}

void tween_init(tween *Tween, float From, float To, float Duration,
                tween_easing Easing)
{
  Tween->From     = From;
  Tween->To       = To;
  Tween->Duration = Duration;
  Tween->Elapsed  = 0.f;
  Tween->Value    = From;
  Tween->Easing   = Easing;
  Tween->Playing  = 0;
  Tween->Finished = 0;
}

void tween_start(tween *Tween)
{
  Tween->Elapsed  = 0.f;
  Tween->Value    = Tween->From;
  Tween->Playing  = 1;
  Tween->Finished = 0;
}

void tween_stop(tween *Tween)
{
  Tween->Playing = 0;
}

void tween_reset(tween *Tween)
{
  Tween->Elapsed  = 0.f;
  Tween->Value    = Tween->From;
  Tween->Playing  = 0;
  Tween->Finished = 0;
}

void tween_update(tween *Tween, float DeltaTime)
{
  float Progress;
  float Eased;

  if (!Tween->Playing)
    return;

  if (DeltaTime < 0.f)
    DeltaTime = 0.f;

  if (Tween->Duration <= 0.f)
  {
    Tween->Elapsed  = 0.f;
    Tween->Value    = Tween->To;
    Tween->Playing  = 0;
    Tween->Finished = 1;
    return;
  }

  Tween->Elapsed += DeltaTime;

  if (Tween->Elapsed >= Tween->Duration)
  {
    Tween->Elapsed  = Tween->Duration;
    Tween->Value    = Tween->To;
    Tween->Playing  = 0;
    Tween->Finished = 1;
    return;
  }

  Progress = Tween->Elapsed / Tween->Duration;
  Eased    = tween_ease(Progress, Tween->Easing);

  Tween->Value = Tween->From + (Tween->To - Tween->From) * Eased;
}

float tween_get_value(tween *Tween)
{
  return Tween->Value;
}

float tween_get_progress(tween *Tween)
{
  if (Tween->Duration <= 0.f)
    return 1.f;

  return tween_clamp(Tween->Elapsed / Tween->Duration, 0.f, 1.f);
}

int tween_is_playing(tween *Tween)
{
  return Tween->Playing;
}

int tween_is_finished(tween *Tween)
{
  return Tween->Finished;
}
