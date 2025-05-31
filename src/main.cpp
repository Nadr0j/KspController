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
  uint32_t lastPressTime;
  bool ledState;
};

struct ControlSystem
{
  uint8_t togglePin;
  bool lastState;
  void (*onchange)(bool newState);
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

void sasOnChange(bool newState)
{
  Serial.print("SAS: ");
  Serial.println(newState ? "ON" : "OFF");
}

void rcsOnChange(bool newState)
{
  Serial.print("RCS: ");
  Serial.println(newState ? "ON" : "OFF");
}

void stageOnRising()
{
  Serial.println("STAGE: staging");
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
    10,    // button pin
    11,    // toggle pin
    12,    // led pin
    false, // button state starts false
    0,     // last press time
    false  // led state - this needs to be updated in setup
};

ControlSystem controlSystems[] = {
    {13, false, sasOnChange}, // state needs to be updated in setup
    {A0, false, rcsOnChange}  // state needs to be updated in setup
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

void initializeStageSystem()
{
  pinMode(stageSystem.buttonPin, INPUT_PULLUP);
  pinMode(stageSystem.togglePin, INPUT_PULLUP);
  pinMode(stageSystem.ledPin, OUTPUT);
  stageSystem.ledState = digitalRead(stageSystem.togglePin) == LOW;
  digitalWrite(stageSystem.ledPin, stageSystem.ledState ? HIGH : LOW);
}

void initializeControlSystems()
{
  for (int i = 0; i < sizeof(controlSystems) / sizeof(controlSystems[0]); i++)
  {
    ControlSystem &controlSystem = controlSystems[i];
    pinMode(controlSystem.togglePin, INPUT_PULLUP);
    controlSystem.lastState = digitalRead(controlSystem.togglePin) == LOW;
    Serial.print("Initialized pin ");
    Serial.print(controlSystem.togglePin);
    Serial.print(" to state: ");
    Serial.println(controlSystem.lastState);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Setup vec buttons and leds
  initializeVecButtons();
  initializeStageSystem();
  initializeControlSystems();
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

void processStageSystem(StageSystem &stageSystem)
{
  bool systemOn = digitalRead(stageSystem.togglePin) == LOW;

  if (stageSystem.ledState != systemOn)
  {
    stageSystem.ledState = systemOn;
    digitalWrite(stageSystem.ledPin, systemOn ? HIGH : LOW);
  }

  if (!systemOn)
  { // staging disabled
    stageSystem.lastButtonState = false;
    return;
  }

  const bool curr = (digitalRead(stageSystem.buttonPin) == LOW);

  if (!stageSystem.lastButtonState &&
      curr &&
      millis() - stageSystem.lastPressTime > DEBOUNCE_MS)
  {
    stageSystem.lastPressTime = millis();
    stageOnRising();
  }

  stageSystem.lastButtonState = curr;
}

void processControlSystems(ControlSystem systems[], size_t count)
{
  for (int i = 0; i < count; i++)
  {
    ControlSystem &sys = controlSystems[i];
    bool newState = digitalRead(controlSystems[i].togglePin) == LOW;

    if (newState != sys.lastState)
    {
      sys.lastState = newState;
      Serial.print("Updated toggle at pin ");
      Serial.print(sys.togglePin);
      Serial.print(" to state ");
      Serial.println(sys.lastState);
    }
  }
}

void loop()
{
  processVecButtonOnLoop();
  processStageSystem(stageSystem);

  const size_t controlSystemCount = sizeof(controlSystems) / sizeof(controlSystems[0]);
  processControlSystems(controlSystems, controlSystemCount);
}
