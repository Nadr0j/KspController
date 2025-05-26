#include <KerbalSimpit.h>
#include "Arduino.h"

void triggerSTB(bool newState)
{
  Serial.print("STB: ");
  Serial.println(newState ? "ON" : "OFF");
}

void triggerTRG(bool newState)
{
  Serial.print("TRG: ");
  Serial.println(newState ? "ON" : "OFF");
}

void triggerPRO(bool newState)
{
  Serial.print("PRO: ");
  Serial.println(newState ? "ON" : "OFF");
}

void triggerRTO(bool newState)
{
  Serial.print("RTO: ");
  Serial.println(newState ? "ON" : "OFF");
}

struct ButtonLedPair
{
  uint8_t buttonPin;
  uint8_t ledPin;
  void (*onchange)(bool newState);
};

struct ToggleSwitch
{
  uint8_t togglePin;
};

struct Button
{
  uint8_t buttonPin;
};

struct Potentiometer
{
  uint8_t potPin;
};

ButtonLedPair vecButtonLedPairs[] = {
    {2, 3, triggerSTB}, // STB
    {4, 5, triggerTRG}, // TRG
    {6, 7, triggerPRO}, // PRO
    {8, 9, triggerRTO}, // RTO
};

ToggleSwitch toggleSwitches[] = {
    {10}, // SAS
    {11}, // RCS
    {12}, // STAGE
};

Button buttons[] = {
    {13}, // STAGE BUTTON
};

Potentiometer pots[] = {
    {A0}, // THROTTLE
    {A1}, // ROLL
    {A2}, // PITCH
    {A3}, // YAW
};

const int numVecButtonLedPairs = sizeof(vecButtonLedPairs) / sizeof(vecButtonLedPairs[0]);
bool vecButtonSystemStates[numVecButtonLedPairs] = {false};
bool vecButtonLastStates[numVecButtonLedPairs] = {true};

// const int numToggleSwitches = sizeof(toggleSwitches) / sizeof(toggleSwitches[0]);
// bool toggleSwitchStates[numToggleSwitches];
// bool toggleSwitchLastStates[numToggleSwitches];

// void initializeToggleButtons()
// {
//   for (int i = 0; i < numToggleSwitches; i++)
//   {
//     pinMode(toggleSwitches[i].togglePin, INPUT_PULLUP);
//     bool toggleIsOn = digitalRead(toggleSwitches[i].togglePin) == LOW; // due to input pullup pressed == LOW
//     toggleSwitchStates[i] = toggleIsOn;
//     toggleSwitchLastStates[i] = toggleIsOn;
//   }
// }

void initializeVecButtons()
{
  for (int i = 0; i < numVecButtonLedPairs; i++)
  {
    pinMode(vecButtonLedPairs[i].buttonPin, INPUT_PULLUP);
    pinMode(vecButtonLedPairs[i].ledPin, OUTPUT);
    digitalWrite(vecButtonLedPairs[i].ledPin, LOW);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...");

  // Setup vec buttons and leds
  initializeVecButtons();
  // initializeToggleButtons();
}

void processVecButton(int i, ButtonLedPair &pair)
{
  bool buttonPressed = digitalRead(pair.buttonPin) == LOW; // due to input pullup pressed == LOW
  bool wasPreviouslyPressed = vecButtonLastStates[i] == LOW;

  // detect rising edge
  if (buttonPressed && !wasPreviouslyPressed)
  {
    vecButtonSystemStates[i] = !vecButtonSystemStates[i]; // toggle LED state due to rising edge detection
    digitalWrite(pair.ledPin, vecButtonSystemStates[i] ? HIGH : LOW);
  };

  pair.onchange(vecButtonSystemStates[i]);             // Call the onchange function with the new state
  vecButtonLastStates[i] = buttonPressed ? LOW : HIGH; // Update last known button state
}

void loop()
{
  // Handle VEC buttons
  for (int i = 0; i < numVecButtonLedPairs; i++)
  {
    ButtonLedPair &pair = vecButtonLedPairs[i];
    processVecButton(i, pair);
  }
}
