#include <KerbalSimpit.h>
#include "Arduino.h"

struct VecButtonSystem
{
  uint8_t buttonPin;
  uint8_t ledPin;
  bool lastButtonState;
  bool systemState;
  void (*onchange)(int i, bool newState, VecButtonSystem vecButtonSystems[]);
};

struct StageSystem
{
  uint8_t buttonPin;
  uint8_t togglePin;
  uint8_t ledPin;
  bool lastButtonState;
};

void stbOnChange(int i, bool newState, VecButtonSystem vecButtonSystems[])
{
  VecButtonSystem &system = vecButtonSystems[i];
  system.systemState = newState;
  digitalWrite(system.ledPin, newState ? HIGH : LOW);

  Serial.print("STB: ");
  Serial.println(newState ? "ON" : "OFF");
}

void trgOnChange(int i, bool newState, VecButtonSystem vecButtonSystems[])
{
  Serial.print("TRG: ");
  Serial.println(newState ? "ON" : "OFF");
}

void proOnChange(int i, bool newState, VecButtonSystem vecButtonSystems[])
{
  Serial.print("PRO: ");
  Serial.println(newState ? "ON" : "OFF");
}

void rtoOnChange(int i, bool newState, VecButtonSystem vecButtonSystems[])
{
  Serial.print("RTO: ");
  Serial.println(newState ? "ON" : "OFF");
}

/*
SYSTEMS
*/

VecButtonSystem vecButtonSystems[] = {
    {2, 3, false, false, stbOnChange}, // STB
    {4, 5, false, false, trgOnChange}, // TRG
    {6, 7, false, false, proOnChange}, // PRO
    {8, 9, false, false, rtoOnChange}, // RTO
};

StageSystem stageSystem = {
    10,   // button pin
    11,   // toggle pin
    12,   // led pin
    false // button state starts false
};

void flipButtonState(int buttonIdx, bool newState)
{
  for (int i = 0; i < sizeof(vecButtonSystems) / sizeof(vecButtonSystems[0]); i++)
  {
    if (i == buttonIdx)
    {
      bool currentSystemState = vecButtonSystems[i].systemState;
      vecButtonSystems[i].onchange(i, !currentSystemState, vecButtonSystems);
    }
    else
    {
      vecButtonSystems[i].onchange(i, false, vecButtonSystems);
    }
  }
}

void initializeVecButtons()
{
  for (int i = 0; i < sizeof(vecButtonSystems) / sizeof(vecButtonSystems[0]); i++)
  {
    pinMode(vecButtonSystems[i].buttonPin, INPUT_PULLUP);
    pinMode(vecButtonSystems[i].ledPin, OUTPUT);
    digitalWrite(vecButtonSystems[i].ledPin, LOW);
  }
}

void initializeStageSystem() {
  pinMode(stageSystem.buttonPin, INPUT_PULLUP);
  pinMode(stageSystem.ledPin, OUTPUT);
  digitalWrite(stageSystem.ledPin, LOW);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Setup vec buttons and leds
  initializeVecButtons();
}

const uint8_t N = sizeof(vecButtonSystems) / sizeof(vecButtonSystems[0]);
const uint32_t DEBOUNCE_MS = 100;
static uint32_t lastEdge[N];

void processVecButtonOnLoop()
{
  for (int i = 0; i < sizeof(vecButtonSystems) / sizeof(vecButtonSystems[0]); i++)
  {
    // true if button on else false
    bool lastButtonState = vecButtonSystems[i].lastButtonState;
    bool currentButtonState = digitalRead(vecButtonSystems[i].buttonPin) == LOW;

    if (!lastButtonState && currentButtonState && (millis() - lastEdge[i] > DEBOUNCE_MS))
    {
      // rising edge case
      lastEdge[i] = millis();
      flipButtonState(i, currentButtonState);
    }

    vecButtonSystems[i].lastButtonState = currentButtonState;
  }
}

void loop()
{
  // Handle VEC buttons
  processVecButtonOnLoop();
}
