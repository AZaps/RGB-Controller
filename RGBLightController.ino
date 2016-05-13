/*
 * RGB Light Controller
 * Version 3.0 
 * 
 * Anthony Zaprzalka
 * 
 * This is pretty basic and uses the ISL29125 sensor breakout board from SparkFun.
 * I have this setup directly in front of a TV which can determine the general output color and
 * change the colors on the lightstrip to match.
 * 
 * SDA -> A4 on Uno
 * SCL -> A5 on Uno
 * 
 * Using analog pins for controlling the color output
 * **Dependent on setup, may require some tinkering!
 * Red -> 11
 * Green -> 9
 * Blue -> 10
 * 
 * Externally power the lightstrip, so the microcontroller doesn't fry!
 */

/* Library Declarations */
#include <Arduino.h>
#include <SFE_ISL29125.h>
#include <Wire.h>

// Declare the sensor object
SFE_ISL29125 RGB_sensor;

// Pin assignment for RGB pins
const int redLight = 11;
const int greenLight = 9;
const int blueLight = 10;

// Sensor Values
unsigned long red = 0;
unsigned long green = 0;
unsigned long blue = 0;

// Previous Reading
unsigned long redPrev = 50;
unsigned long greenPrev = 50;
unsigned long bluePrev = 50;

// Averages
unsigned long redAverage = 0;
unsigned long greenAverage = 0;
unsigned long blueAverage = 0;

// Total Average
int averageCounter = 0;

// Difference between colors
unsigned long redDiff = 0;
unsigned long greenDiff = 0;
unsigned long blueDiff = 0;

unsigned long largestDiff = 0;

// If previous color was larger or not
bool redGreater;
bool greenGreater;
bool blueGreater;

// Colors to be outputted to controller
unsigned int redOutput;
unsigned int greenOutput;
unsigned int blueOutput;

void setup() {
  
  Serial.begin(115200);

  // Initialize the ISL29125 with simple configuration so it starts sampling
  if (RGB_sensor.init())
  {
    Serial.println("\nSensor Initialization Successful\n");
    lightStart();
  } else {
    Serial.println("\nUnsuccessful\n");
    // Flash Red
    for (int i = 0; i < 2; i++) {
      outputColor(255,0,0);
      delay(1000);
    }
  }

  // Configure the sensor
  RGB_sensor.config(CFG1_MODE_RGB | CFG1_375LUX | CFG1_12BIT, CFG2_IR_OFFSET_ON | CFG2_IR_ADJUST_MID, CFG_DEFAULT);
  
} // End of setup()


void loop() {
  // Read the sensor values
  red = RGB_sensor.readRed();
  green = RGB_sensor.readGreen();
  blue = RGB_sensor.readBlue();

  // DEGBUG STATEMENTS
  Serial.print("Immediate Sensor Readouts... ");
  Serial.print("red = ");Serial.print(red); Serial.print(" | ");
  Serial.print("green = ");Serial.print(green); Serial.print(" | ");
  Serial.print("blue = ");Serial.println(blue);
  

  // Average track
  redAverage = redAverage + red;
  greenAverage = greenAverage + green;
  blueAverage = blueAverage + blue;

  averageCounter++;

  if (averageCounter == 10) {
    
    red =  16.5 * (redAverage/averageCounter);
    green = 8.8 * (greenAverage/averageCounter);
    blue = 8.8 * (blueAverage/averageCounter);
    
    // DEBUG STATEMENTS
    Serial.println();
    Serial.print("AVERAGE... ");
    Serial.print("red = ");Serial.print(redAverage); Serial.print(" | ");
    Serial.print("green = ");Serial.print(greenAverage); Serial.print(" | ");
    Serial.print("blue = ");Serial.println(blueAverage);
   
    
    // DEBUG STATEMENTS
    Serial.print("12 bit average color... ");
    Serial.print("red = ");Serial.print(red); Serial.print(" | ");
    Serial.print("green = ");Serial.print(green); Serial.print(" | ");
    Serial.print("blue = ");Serial.println(blue);
   

    // Map the values from 0 - 4095 (12 bit) to 0 - 255 (8 bit)
    red = mapToEightBit(red);
    green = mapToEightBit(green);
    blue = mapToEightBit(blue);

    if (red > 255) {
      red = 255;
    }

    if (green > 255) {
      green = 255;
    }

    if (blue > 255) {
      blue = 255;
    }
    // DEBUG STATEMENTS
    Serial.print("8 bit average color... ");
    Serial.print("red = ");Serial.print(red); Serial.print(" | ");
    Serial.print("green = ");Serial.print(green); Serial.print(" | ");
    Serial.print("blue = ");Serial.println(blue);

    Serial.print("Previous Color... ");
    Serial.print("red = ");Serial.print(redPrev); Serial.print(" | ");
    Serial.print("green = ");Serial.print(greenPrev); Serial.print(" | ");
    Serial.print("blue = ");Serial.println(bluePrev);
    

    // Output color to controller
    outputColorDuration(red, green, blue);

    // Set previous values
    redPrev = red;
    greenPrev = green;
    bluePrev = blue;
    
    // Reset values
    averageCounter = 0;
    red = 0;
    green = 0;
    blue = 0;
    redAverage = 0;
    greenAverage = 0;
    blueAverage = 0;

    // DEBUG STATEMENT
    Serial.println("\n---------------------------------------- END ----------------------------------------\n");
  }
  
} // End of loop()

void outputColor(int r, int g, int b) {
  analogWrite(redLight, r);
  analogWrite(greenLight, g);
  analogWrite(blueLight, b);
}

void lightStart() {
  // Fade in white
  for (int i = 0; i < 256; i++) {
    outputColor(i, i, i);
    delay(10);
  }
  // Fade out white
  for (int i = 255; i > 50; i--) {
    outputColor(i, i, i);
    delay(10);
  }
  // Also check if correctly plugged in 
  outputColor(255, 0, 0);
  delay(1000);
  outputColor(0, 255, 0);
  delay(1000);
  outputColor(0, 0, 255);
  delay(1000);
  outputColor(0,0,0);
  delay(500);
}

unsigned long mapToEightBit(unsigned long unmappedValue) {
  return unmappedValue = map(unmappedValue, 0, 4095, 0, 255);
}

void outputColorDuration(unsigned long redOut, unsigned long greenOut, unsigned long blueOut) {
  // Determine differences between colors
  // RED
  if (redOut > redPrev) {
    redDiff = redOut - redPrev;
    redGreater = true;
  } else {
    redDiff = redPrev - redOut;
    redGreater = false;
  }
  redOutput = redPrev;

  // GREEN
  if (greenOut > greenPrev) {
    greenDiff = greenOut - greenPrev;
    greenGreater = true;
  } else {
    greenDiff = greenPrev - greenOut;
    greenGreater = false;
  }
  greenOutput = greenPrev;

  // BLUE
  if (blueOut > bluePrev) {
    blueDiff = blueOut - bluePrev;
    blueGreater = true;
  } else {
    blueDiff = bluePrev - blueOut;
    blueGreater = false;
  }
  blueOutput = bluePrev;

  // Determine the greatest difference to be used as a increment
  // Red vs. Green
  if (redDiff > greenDiff) {
    largestDiff = redDiff;
  } else {
    largestDiff = greenDiff;
  }
  // (Red vs. Green) vs. Blue
  if (largestDiff < blueDiff) {
    largestDiff = blueDiff;
  }


  
  // DEBUG STATEMENT
  Serial.print("The largest difference is... ");
  Serial.println(largestDiff);
  Serial.println();

  
  for (largestDiff != 0; largestDiff--;) {
    // RED
    // Go until the difference is 0
    if (redDiff !=0) {
      if (redGreater) {
        // red is greater than redPrev
        redOutput++;
        redDiff--;
      } else {
        // red is less than redPrev
        redOutput--;
        redDiff--;
      }
    }

    // GREEN
    // Go until the difference is 0
    if (greenDiff !=0) {
      if (greenGreater) {
        // green is greater than greenPrev
        greenOutput++;
        greenDiff--;
      } else {
        // green is less than greenPrev
        greenOutput--;
        greenDiff--;
      }
    }

    // BLUE
    // Go until the difference is 0
    if (blueDiff !=0) {
      if (blueGreater) {
        // blue is greater than bluePrev
        blueOutput++;
        blueDiff--;
      } else {
        // blue is less than bluePrev
        blueOutput--;
        blueDiff--;
      }
    }
    
    // DEBUG STATEMENTS
    Serial.print("Color to be outputted... ");
    Serial.print("red = ");Serial.print(redOutput); Serial.print(" | ");
    Serial.print("green = ");Serial.print(greenOutput); Serial.print(" | ");
    Serial.print("blue = ");Serial.println(blueOutput);
    
    // Output the color to the controller
    outputColor(redOutput, greenOutput, blueOutput);
    delay(7);
  }
}
















