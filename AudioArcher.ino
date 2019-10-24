// AudioArcher
// 4WWB0 - Group 217
// Usage of the PVision library with instructions by www.stephenhobley.com

#include <Wire.h>
#include <PVision.h>
#include <Arduino.h>

PVision ircam;  // Create PVision object
byte result;

// Set up IR sensor
void setup()
{
  Serial.begin(9600);
  ircam.init();
  for (int i=0;i<8;i++){
    pinMode(i,OUTPUT);
  }
}

// Define sinewave for quicker sound output (no calc needed)
byte sine[] = {127, 134, 142, 150, 158, 166, 173, 181, 188, 195, 201, 207, 213,
219, 224, 229, 234, 238, 241, 245, 247, 250, 251, 252, 253, 254, 253, 252, 251,
250, 247, 245, 241, 238, 234, 229, 224, 219, 213, 207, 201, 195, 188, 181, 173,
166, 158, 150, 142, 134, 127, 119, 111, 103, 95, 87, 80, 72, 65, 58, 52, 46, 40,
34, 29, 24, 19, 15, 12, 8, 6, 3, 2, 1, 0, 0, 0, 1, 2, 3, 6, 8, 12, 15, 19, 24,
29, 34, 40, 46, 52, 58, 65, 72, 80, 87, 95, 103, 111, 119,};
int loc_x[] = {0,0,0,0,0};
int loc_y[] = {0,0,0,0,0};

// Main loop
void loop()
{
  //Move the values in the array to the end to make space for the new value
  for(int i = 4; i>=1; i--){
    loc_x[i]=loc_x[i-1];
    loc_y[i]=loc_y[i-1];
  }
  //Initialize the first space in the array to 0
  loc_x[0]=0;
  loc_y[0]=0;

  //Get information from the camera
  result = ircam.read();

  //Store the first led detected in the first space of the array
  if (result & BLOB1){
    loc_x[0] = ircam.Blob1.X;
    loc_y[0] = ircam.Blob1.Y;
  }
  //If a second led is detected store the average of the 2 leds in the array (overwrite)
  if (result & BLOB2){
    loc_x[0] = (loc_x[0] + ircam.Blob2.X)/2;
    loc_y[0] = (loc_y[0] + ircam.Blob2.Y)/2;
  }

  int loc_x_avg = 0;
  int loc_y_avg = 0;

  // Calculate average locations
  for(int i=0; i<=4;i++)
  {
      loc_x_avg = loc_x_avg + loc_x[i];
      loc_y_avg = loc_y_avg + loc_y[i];
  }
  int s1 = (loc_x_avg)/4;
  int s2 = (loc_y_avg)/4;

  loc_x_avg = s1;
  loc_y_avg = s2;

  // Map X/Y position values to 0..100 scale (0..Far away, 100..at center)
  int x_percent = 0;
  int y_percent = 0;
  x_percent = map(abs(512-loc_x_avg),0,512,0,100);
  y_percent = 100-map(abs(384-loc_y_avg),0,384,0,100);

  // Set mute to default off (pulse control of the signal)
  bool mute = false;

  // Frequency calculations (for pitch adjustments - Y direction)
  int MAX_FREQUENCY = 2000; // Maximum frequency (pitch change!) at center
  int MIN_FREQUENCY = 200; // Minimum frequency (pitch change!) (out of range)
  int frequency = (((MAX_FREQUENCY - MIN_FREQUENCY) * y_percent) / 100) + MIN_FREQUENCY; // Calc freq based on y-value

  // Muting calculations (for sound pulse adjustments - X direction)
  int MAX_PAUSE = 80; // Value range [0,100], defining how many percent of the refresh_time the pause should max last
  int MIN_PAUSE = 10; // Value range [0,100], defining how many percent of the refresh_time the pause should min last
  int pause = (((MAX_PAUSE - MIN_PAUSE) * x_percent) / 100) + MIN_PAUSE;

  // Calculating sample durations and periods
  int SAMPLE_NUMBER = 100;  // Number of samples for a full sine period (related to the sine[] array)
  int step_duration = 1000000 / (frequency * sample_number);   // Duration in us that each sine value is active on the port (including us to s conversion)
  int refresh_time = 250;     // New signal (recalculation / new cycle) after refresh time in ms
  int max_periods   = (refresh_time * 1000) / (step_duration * sample_number);    // Number of full sine periods to cycle through before value update

  //Audio generation loop
  for(int periods = 0; periods < max_periods; periods++){
    for (int t=0; t < sample_number; t++){
      if(periods <= (max_periods * pause) / 100){
        mute = true;
      }else{
        mute = false;
      }
      if(mute){
        PORTD = sine[0];
      }else{
        PORTD = sine[t];
      }
      delayMicroseconds(step_duration);
  }
  }
}
