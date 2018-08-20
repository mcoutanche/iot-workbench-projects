# 1 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino"
# 1 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino"
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/door-monitor?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode
# 5 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2
# 6 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2
# 7 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2
# 8 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2
# 9 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2
# 10 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 2





// Audio 

AudioClass& Audio = AudioClass::getInstance();
const int AUDIO_SIZE = 32000 * 3 + 45;

int lastButtonAState;
int buttonAState;
char *audioBuffer;
int totalSize;
int monoSize;

// The magnetometer sensor
static DevI2C *i2c;
static LIS2MDLSensor *lis2mdl;

// Data from magnetometer sensor
static int axes[3];
static int base_x;
static int base_y;
static int base_z;

// Indicate whether the magnetometer sensor has been initialized
static bool initialized = false;

// The open / close status of the door
static bool preOpened = false;

// Indicate whether DoorMonitorSucceed event has been logged
static bool telemetrySent = false;

// Indicate whether WiFi is ready
static bool hasWifi = false;

// Indicate whether IoT Hub is ready
static bool hasIoTHub = false;

// Indicate whether Audi is ready
static bool hasAudio = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWiFi()
{
  Screen.print(2, "Connecting...");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

static void AudioSetup()
{
  //Setup Audio
  Screen.clean();
  Screen.print(1, "Press A to record", true);

  while(1)
  {
    buttonAState = digitalRead(USER_BUTTON_A);
    if (buttonAState == 0x0 && lastButtonAState == 0x1)
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
      hasAudio = true;
      break;
    }
    lastButtonAState = buttonAState;
  }
    delay(10);
}

static void InitMagnetometer()
{
  Screen.clean();
  Screen.print(2, "Initializing...");
  i2c = new DevI2C(D14, D15);
  lis2mdl = new LIS2MDLSensor(*i2c);
  lis2mdl->init(
# 121 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino" 3 4
               __null
# 121 "c:\\Users\\Michael.Coutanche\\Documents\\GitHub\\iot-workbench-projects\\IoTproject-002\\Device\\DoorMonitor.ino"
                   );

  lis2mdl->getMAxes(axes);
  base_x = axes[0];
  base_y = axes[1];
  base_z = axes[2];

  int count = 0;
  int delta = 10;
  char buffer[20];
  while (true)
  {
    delay(1000);
    lis2mdl->getMAxes(axes);

    // Waiting for the data from sensor to become stable
    if (abs(base_x - axes[0]) < delta && abs(base_y - axes[1]) < delta && abs(base_z - axes[2]) < delta)
    {
      count++;
      if (count >= 5)
      {
        // Done
        Screen.print(0, "Monitoring...");
        break;
      }
    }
    else
    {
      count = 0;
      base_x = axes[0];
      base_y = axes[1];
      base_z = axes[2];
    }
    sprintf(buffer, "      %d", 5 - count);
    Screen.print(1, buffer);
  }
}

void CheckMagnetometerStatus()
{
  char *message;
  int delta = 30;
  int count = 0;
  bool curOpened = false;
  if (abs(base_x - axes[0]) < delta && abs(base_y - axes[1]) < delta && abs(base_z - axes[2]) < delta)
  {
    Screen.print(0, "Door closed");
    message = "{\"DoorStatus\":\"Closed\"}";
    curOpened = false;
   }
  else
  {
    Screen.print(0, "Door opened");
    message = "{\"DoorStatus\":\"Opened\"}";
    curOpened = true;
    play();
  }
  // send message when status change
  if (curOpened != preOpened)
  {
    if (DevKitMQTTClient_SendEvent(message))
    {
      if (!telemetrySent)
      {
        telemetrySent = true;
        LogTrace("DoorMonitorSucceed", "ver=1.0");
      }
    }
    preOpened = curOpened;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{

  Screen.init();
  Screen.print(0, "DoorMonitor");
  Screen.print(2, "Initializing...");
  Screen.print(3, " > Serial");
  Serial.begin(115200);

  pinMode(PC_13, 0x2);
  Serial.begin(115200);

  Serial.println("Helloworld in Azure IoT DevKits!");

  // initialize the button pin as a input
  pinMode(USER_BUTTON_A, 0x1);
  lastButtonAState = digitalRead(USER_BUTTON_A);

  // Setup your local audio buffer
  audioBuffer = (char *)malloc(AUDIO_SIZE + 1);
  memset(audioBuffer, 0x0, AUDIO_SIZE);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWiFi();
    if (!hasWifi)
  {
    return;
  }
  LogTrace("DoorMonitor", "ver=1.0");

  // IoT hub
  Screen.print(3, " > IoT Hub");
  DevKitMQTTClient_SetOption("MiniSolution", "DoorMonitor");
  if (!DevKitMQTTClient_Init())
  {
    Screen.clean();
    Screen.print(0, "DoorMonitor");
    Screen.print(2, "No IoT Hub");
    hasIoTHub = false;
    return;
  }
  hasIoTHub = true;

    // Setup Audio
  Screen.print(3, " > Audio Setup");
  hasAudio = false;

  AudioSetup();



 Screen.print(3, " > Magnetometer");
InitMagnetometer();


}

void loop()
{
  if (hasWifi && hasIoTHub && hasAudio)
  {
    // Get data from magnetometer sensor
    lis2mdl->getMAxes(axes);
    Serial.printf("Axes: x - %d, y - %d, z - %d\r\n", axes[0], axes[1], axes[2]);

    char buffer[50];

    sprintf(buffer, "x:  %d", axes[0]);
    Screen.print(1, buffer);

    sprintf(buffer, "y:  %d", axes[1]);
    Screen.print(2, buffer);

    sprintf(buffer, "z:  %d", axes[2]);
    Screen.print(3, buffer);

    CheckMagnetometerStatus();
  }
  delay(1000);
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
}
