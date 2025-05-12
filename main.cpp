#include <Arduino.h>

/*
To Do:
- Fix calibration and upper/lower limits for each motor


*/

/*------------------CONSTANTS------------------*/
const int pwmPin = 13;
const int pwmFreq = 50;
const int pwmResolution = 16; // 16-bit resolution (0-65536)
const int LED_BUILTIN = 2;

/*------------------SETTINGS-------------------*/

// Offsets
const int motor_0_offset_90  = 5775.15;
const int motor_0_offset_180 = 8825.25;
const int motor_1_offset_90  = 4420;
const int motor_1_offset_180 = 7850;
const int motor_2_offset_90  = 3063.95;
const int motor_2_offset_180 = 6400;
const int motor_3_offset_90  = 5097.35;
const int motor_3_offset_180 = 8350;
const int motor_4_offset_90  = 5400;
const int motor_4_offset_180 = 8000;

// How much is the angle off?
const int motor_0_offset = 45;
const int motor_1_offset = 5;
const int motor_2_offset = -35;
const int motor_3_offset = 25;
const int motor_4_offset = 0;

int offset_array[5] = {motor_0_offset, motor_1_offset, motor_2_offset, motor_3_offset, motor_4_offset};
int offset_array_90[5] = {motor_0_offset_90, motor_1_offset_90, motor_2_offset_90, motor_3_offset_90, motor_4_offset_90};
int offset_array_180[5] = {motor_0_offset_180, motor_1_offset_180, motor_2_offset_180, motor_3_offset_180, motor_4_offset_180};

/*------------------VARIABLES------------------*/
int incomingdata = 0;
bool LEDstate = HIGH;

/*------------------FUNCTIONS------------------*/

void move_to(int PWM_channel, int degrees)
{
    degrees = degrees + offset_array[PWM_channel];
    int bits = int(33.89 * degrees + 1200);
    ledcWrite(PWM_channel, bits);
}

void move_to_new(int PWM_channel, int degrees)
{
    int offset_90 = offset_array_90[PWM_channel];
    int offset_180 = offset_array_180[PWM_channel];
    // Point slop linear formula
    int bits = int(((offset_180 - offset_90)/90) * (degrees - 90) + offset_90);
    ledcWrite(PWM_channel, bits);
}

void move_to_bits(int PWM_channel, int bits)
{
    ledcWrite(PWM_channel, bits);
}

void jork_it()
{
    move_to_new(2, 180);
    for(;;)
    {
        move_to_new(1, 90);
        move_to_new(4, 0); 
        move_to_new(2, 90);
        delay(200);
        move_to_new(1, 180);
        move_to_new(4, 180);
        delay(200);
    }
}

void slow_move_up(int PWM_channel, int angle) {
    for (int i = 90; i < angle; i++)
    {
        move_to_new(PWM_channel, i);
        delay(17); // Adjust the delay for speed
    }
}

void slow_move_down(int PWM_channel, int angle) {
    for (int i = angle; i >= 90; i--)
    {
        move_to_new(PWM_channel, i);
        delay(17); // Adjust the delay for speed
    }
}

void grab() {

    move_to_new(4, 0);
    slow_move_up(1, 150);
    slow_move_up(2, 150);
    slow_move_up(4, 180);

    delay(3000);
    move_to_new(4, 0);
    slow_move_down(2, 150);
    slow_move_down(1, 150);
}

String readUART()
{
    String data = "";
    while (Serial2.available() > 0)
    {
        /*
        char c = Serial2.read();
        data += c;
        delay(10); // Small delay to ensure all data is read
        */

        data = Serial2.readStringUntil('\n'); // Read until newline character
        delay(10);
    }
    return data;
}

int read_cam_data()
{
    // Read serial data
    //Serial.println("Reading camera data...");

    // Debug Test:

    if (Serial2.available() > 0) {
        
        int incomingdata = readUART().toInt();
        //Serial.print("Data: ");
        //Serial.println(incomingdata);
        return incomingdata;
    }
    else {
        return int(100);
    }
}

void move_to_center(int angle, int x) {
    // Keep moving until the object is centered

    // Debug
    // Serial.println("Moving to center...");

    int counter = 0;

    for(;;)
    {

        // Can is out of bounds
        if (x == 100)
        {

            if (counter > 500)
            {
                //Serial.printf("Object out of bounds...");
                // Go back to idle
                break;
            }

            x = read_cam_data();

            counter++;
        }
        else if (x == 21)
        {
            // Centered
            //Serial.println("Object in the middle...");
            grab();
            move_to_new(0, angle);
            delay(100);
            x = read_cam_data();
            counter = 0;
        }
        else if (x > 21)                                       // Can is to the right
        {
            // Move left
            //Serial.println("Object to the right...");
            angle = angle + 1;
            move_to_new(0, angle);
            delay(300);
            x = read_cam_data();
            counter = 0;
        }
        else if (x < 21)                                       // Can is to the left
        {
            // Move right
            //Serial.println("Object to the left...");
            angle = angle - 1;
            move_to_new(0, angle);
            delay(300);
            x = read_cam_data();
            counter = 0;
        }
        else 
        {
            break;
        }
    }

    /*
    if (camera_data == "*")
    {
        // DEBUG
        for (;;)
        {
            move_to_new(0, 0);
        }
        return;
    }
    else
    {
        if (x <= 53 && x >= 43)
        {
            // Centered... START MANUAL MODE
            return;
        }
        else if (x > 53)
        {
            // Move left
            move_to_new(0, angle - 1);
            return;
        }
        else if (x < 43)
        {
            // Move Rights
            move_to_new(0, angle + 1);
            return;
        }
        else
        {
            return;
        }
    }
    */

}


void idle()
{
    // Keep Circuling until object detected

    //Serial.println("Idle mode...");

    for (;;) {
        for(int i = 50; i < 130; i++)
        {

            // Serial.printf("Angle: %d \n", i);

            move_to_new(0, i);
            delay(17);

            int camera_data = read_cam_data();

            if (camera_data != 100)
            {
                // Serial.println("Object detected!");
                move_to_center(i, camera_data);
            }

            digitalWrite(LED_BUILTIN, HIGH);

        }
        for (int i = 130; i > 50; i--)
        {

            // Serial.printf("Angle: %d \n", i);

            move_to_new(0, i);
            delay(17);

            int camera_data = read_cam_data();

            if (camera_data != 100)
            {
                // Serial.println("Object detected!");
                move_to_center(i, camera_data);
            }

            digitalWrite(LED_BUILTIN, LOW);

        }
    }
    
}


void setup() 
{
    /*--------------PWM SETUP----------------*/

    // Motor 0, GPIO 15
    ledcSetup(0, pwmFreq, pwmResolution);
    ledcAttachPin(15, 0);
    // Motor 1, GPIO 19
    ledcSetup(1, pwmFreq, pwmResolution);
    ledcAttachPin(19, 1);
    // Motor 2, GPIO 4
    ledcSetup(2, pwmFreq, pwmResolution);
    ledcAttachPin(4, 2);
    // Motor 3, GPIO 5
    ledcSetup(3, pwmFreq, pwmResolution);
    ledcAttachPin(5, 3);
    // Motor 4, GPIO 18
    ledcSetup(4, pwmFreq, pwmResolution);
    ledcAttachPin(18, 4);


    /*--------------SERIAL SETUP----------------*/
    Serial.begin(9600); // Initialize serial communication at 9600 baud rate
    Serial.println("PWM Example");

    Serial2.begin(115200, SERIAL_8N1, 16, 17); // RX on GPIO16, TX on GPIO17
    Serial.println("UART Initialized");

    // Configure the built-in LED pin as an output
    pinMode(LED_BUILTIN, OUTPUT);
    
}

// Manually Move the arm using serial commands
void manual()
{

    for (;;) 
    {
        if (Serial.available() > 0)
        {
            int motor_num;

            // Input which motor to move
            motor_num = Serial.parseInt();
            Serial.printf("Motor Selected: %d \n", motor_num);
                
            for(;;)
            {
                // Wait for another input
                // Get angle
                if (Serial.available() > 0)
                {
                    int angle;
                    angle = Serial.parseInt();
                    Serial.printf("Bits Selected: %d \n", angle);
                    move_to_new(motor_num, angle);
                    break;
                }
            }

            // LED blink
            digitalWrite(LED_BUILTIN, LEDstate);
            LEDstate = !LEDstate;
        }
    }

}

void loop() 
{
    //manual();

    idle();

}