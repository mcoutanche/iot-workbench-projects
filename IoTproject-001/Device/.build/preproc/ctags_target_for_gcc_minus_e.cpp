# 1 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-001\\Device\\device.ino"
# 1 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-001\\Device\\device.ino"
# 2 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-001\\Device\\device.ino" 2
# 3 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-001\\Device\\device.ino" 2
# 4 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-001\\Device\\device.ino" 2

AudioClass& Audio = AudioClass::getInstance();
const int AUDIO_SIZE = 32000 * 3 + 45;

int lastButtonAState;
int buttonAState;
int lastButtonBState;
int buttonBState;
char *audioBuffer;
int totalSize;
int monoSize;

void setup(void)
{
  pinMode(PC_13, 0x2);
  Serial.begin(115200);

  Serial.println("Helloworld in Azure IoT DevKits!");

  // initialize the button pin as a input
  pinMode(USER_BUTTON_A, 0x1);
  lastButtonAState = digitalRead(USER_BUTTON_A);
  pinMode(USER_BUTTON_B, 0x1);
  lastButtonBState = digitalRead(USER_BUTTON_B);

  // Setup your local audio buffer
  audioBuffer = (char *)malloc(AUDIO_SIZE + 1);
  memset(audioBuffer, 0x0, AUDIO_SIZE);
}

void loop(void)
{
  printIdleMessage();

  while (1)
  {
    buttonAState = digitalRead(USER_BUTTON_A);
    buttonBState = digitalRead(USER_BUTTON_B);

    if (buttonAState == 0x0 && lastButtonAState == 0x1)
    {
      record();
    }

    if (buttonBState == 0x0 && lastButtonBState == 0x1)
    {
      play();
    }

    lastButtonAState = buttonAState;
    lastButtonBState = buttonBState;
  }

  delay(10);
}

void printIdleMessage()
{
  Screen.clean();
  Screen.print(0, "AZ3166 Audio:  ");
  Screen.print(1, "Press A to record", true);
  Screen.print(2, "Press B to play", true);
}

void record()
{
  // Re-config the audio data format
  Audio.format(8000, 16);
  Audio.setVolume(80);

  Serial.println("start recording");
  Screen.clean();
  Screen.print(0, "Start recording");

  // Start to record audio data
  Audio.startRecord(audioBuffer, AUDIO_SIZE);

  // Check whether the audio record is completed.
  while (digitalRead(USER_BUTTON_A) == 0x0 && Audio.getAudioState() == AUDIO_STATE_RECORDING)
  {
    delay(10);
  }
  Audio.stop();

  Screen.clean();
  Screen.print(0, "Finish recording");
  totalSize = Audio.getCurrentSize();
  Serial.print("Recorded size: ");
  Serial.println(totalSize);

  printIdleMessage();
}

void play()
{
  Screen.clean();
  Screen.print(0, "Start playing");
  Audio.startPlay(audioBuffer, totalSize);

  while (Audio.getAudioState() == AUDIO_STATE_PLAYING)
  {
    delay(10);
  }

  Screen.print(0, "Stop playing");
  printIdleMessage();
}
