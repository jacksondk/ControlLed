#ifndef CONTROLLED_PROGRAM
#define CONTROLLED_PROGRAM

#include <pixeltypes.h>
#include <vector>

struct ProgramSetting
{
  ProgramSetting(uint32 ms, CRGB color1, CRGB color2) : ms(ms), color1(color1), color2(color2) {}
  uint32 ms;
  CRGB color1;
  CRGB color2;
};

struct Program
{
  int current_setting_index;
  int current_setting_duration;
  std::vector<ProgramSetting> settings;

  bool updateNeeded;

  Program()
  {
    Reset();
  }

  /** Reset state and delete program */
  void Reset()
  {
    current_setting_duration = 0;
    current_setting_index = 0;
    settings.clear();
  };

  /** Append a program setting to the program */
  void AddProgramSetting(uint32 ms, CRGB color1, CRGB color2)
  {
    settings.push_back(ProgramSetting(ms, color1, color2));
    updateNeeded = true;
  }

  /** Tick the program 
   * 
   * Returns true if the program has changed to a new color settng 
   * and te LEDs need to be updated
   */
  bool Tick(int ms)
  {
    current_setting_duration += ms;
    if (settings[current_setting_index].ms < current_setting_duration)
    {
      current_setting_duration = 0;
      current_setting_index = (current_setting_index + 1) % settings.size();
      updateNeeded = true;
    }
    if (updateNeeded)
    {
      updateNeeded = false;
      return true;
    }
    return false;
  }

  CRGB Color1()
  {
    return settings[current_setting_index].color1;
  }

  CRGB Color2()
  {
    return settings[current_setting_index].color2;
  }
};

#endif