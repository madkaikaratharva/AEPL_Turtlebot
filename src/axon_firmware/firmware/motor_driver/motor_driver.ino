#include <PID_v1.h>

// CONFIGURATION CONSTANTS
// -- Hardware Specific Constraints --
const double TICKS_PER_REV = 770.0;
const double TICKS_TO_RADS = (2*PI)/TICKS_PER_REV;

// -- Pin Configurations --
// Right Motor
const byte L298N_enA = 9;
const byte L298N_in1 = 12;
const byte L298N_in2 = 13;

// Left Motor
const byte L298N_in3 = 7;
const byte L298N_in4 = 8;
const byte L298N_enB = 11;

// Wheel Encoders
//const byte right_encoder_phaseA = 3;  // Interrupt Pin
//const byte right_encoder_phaseB = 5;
const byte right_encoder_phaseA = 2;  // Interrupt Pin
const byte right_encoder_phaseB = 4;


//const byte left_encoder_phaseA = 2;   // Interrupt Pin
//const byte left_encoder_phaseB = 4;
const byte left_encoder_phaseA = 3;   // Interrupt Pin
const byte left_encoder_phaseB = 5;

// GLOBAL VARIABLES
// -- Encoder Counters --
volatile long right_encoder_count = 0;
volatile long left_encoder_count = 0;

// -- Timer --
unsigned long last_millis = 0;
const unsigned long loop_interval = 20;

// -- Control States --
double right_wheel_target_vel = 0.0;
double left_wheel_target_vel = 0.0;

// -- Direction Tracking --
bool right_wheel_forward = true;
bool left_wheel_forward = true;

char right_wheel_dir = 'p';
char left_wheel_dir = 'p';

double right_wheel_vel_signed = 0.0;
double left_wheel_vel_signed = 0.0;

// -- Low Pass Filter --
double alpha = 0.4;
double right_wheel_vel_filtered = 0.0;
double right_wheel_vel_filtered_last = 0.0;
double left_wheel_vel_filtered = 0.0;
double left_wheel_vel_filtered_last = 0.0;

// -- Ramped Setpoints --
double r_setpoint = 0.0;
double l_setpoint = 0.0;
double max_accel = 0.5;

// -- PID --
double right_wheel_pwm = 0.0;
double left_wheel_pwm = 0.0;

double kp_r = 11.5;
double ki_r = 0.5;
double kd_r = 0.0;

double kp_l = 11.5;
double ki_l = 0.5;
double kd_l = 0.0;

// -- Initialize PID --
PID rightPID(&right_wheel_vel_filtered, &right_wheel_pwm, &r_setpoint, kp_r, ki_r, kd_r, DIRECT);
PID leftPID(&left_wheel_vel_filtered, &left_wheel_pwm, &l_setpoint, kp_l, ki_l, kd_l, DIRECT);

// -- Serial Parsing --
const byte MAX_CHARS = 32;
char serial_bytes[MAX_CHARS];
bool new_data = false;


// Noise reduction params
unsigned long right_tick_last_micros = 0;
unsigned long left_tick_last_micros = 0;

void setup() 
{
  // PIN INITIALIZATION
  // Right Motor
  pinMode(L298N_enA, OUTPUT);
  pinMode(L298N_in1, OUTPUT);
  pinMode(L298N_in2, OUTPUT);

  // Left Motor
  pinMode(L298N_enB, OUTPUT);
  pinMode(L298N_in3, OUTPUT);
  pinMode(L298N_in4, OUTPUT);

  // Encoder Pins
  pinMode(right_encoder_phaseA, INPUT_PULLUP);
  pinMode(right_encoder_phaseB, INPUT_PULLUP);

  pinMode(left_encoder_phaseA, INPUT_PULLUP);
  pinMode(left_encoder_phaseB, INPUT_PULLUP);

  // INTERRUPT INITIALIZATION
  attachInterrupt(digitalPinToInterrupt(right_encoder_phaseA), rightEncoderCallback, CHANGE);
  attachInterrupt(digitalPinToInterrupt(left_encoder_phaseA), leftEncoderCallback, CHANGE);

  // PID INITIALIZATION
  rightPID.SetMode(AUTOMATIC);
  rightPID.SetOutputLimits(0.0, 255.0);
  leftPID.SetMode(AUTOMATIC);
  leftPID.SetOutputLimits(0.0, 255.0);

  // MOTOR INITIALIZATION
  // Right Motor (Default: Forward)
  digitalWrite(L298N_in1, HIGH);
  digitalWrite(L298N_in2, LOW);

  // Left Motor (Default: Forward)
  digitalWrite(L298N_in3, HIGH);
  digitalWrite(L298N_in4, LOW);

  // SERIAL PORT INITIALIZATION
  Serial.begin(115200);

}

void loop() 
{
  // Read Serial Data
  readSerial();

  // Process Serial Data
  if(new_data)
  {
    parseData();
  }

  // -- Control Loop --
  unsigned long current_millis = millis();
  if(current_millis - last_millis >= loop_interval)
  {
    // Loop time (in seconds)
    double dt = (current_millis - last_millis) / 1000.0;
    last_millis = current_millis;

    // -- Read Encoder Data --
    noInterrupts(); // Pause interrupts
    long right_wheel_ticks = right_encoder_count;
    long left_wheel_ticks = left_encoder_count;
    right_encoder_count = 0;
    left_encoder_count = 0;
    interrupts(); // Unpause interrupts

    // -- Raw wheel speeds --
    right_wheel_vel_signed = (right_wheel_ticks * TICKS_TO_RADS) / dt;
    left_wheel_vel_signed = (left_wheel_ticks * TICKS_TO_RADS) / dt;

    // Magnitude
    double right_wheel_vel_raw = fabs(right_wheel_vel_signed);
    double left_wheel_vel_raw = fabs(left_wheel_vel_signed);

    //
    if(right_wheel_vel_raw > 20.0)
    {
      right_wheel_vel_raw = 0.0;
    }
    if(left_wheel_vel_raw > 20.0)
    {
      left_wheel_vel_raw = 0.0;
    }

    // -- Low Pass Filter --
    right_wheel_vel_filtered = alpha * right_wheel_vel_raw + (1.0 - alpha) * (right_wheel_vel_filtered_last);
    left_wheel_vel_filtered = alpha * left_wheel_vel_raw + (1.0 - alpha) * (left_wheel_vel_filtered_last);

    // -- Update Filter History --
    right_wheel_vel_filtered_last = right_wheel_vel_filtered;
    left_wheel_vel_filtered_last = left_wheel_vel_filtered;

    // -- Acceleration Ramp. Induces damping --
    updateRamp(r_setpoint, right_wheel_target_vel, max_accel);
    updateRamp(l_setpoint, left_wheel_target_vel, max_accel);

    // -- Handle Direction Switch --
    motorDirectionSwitch(right_wheel_dir, right_wheel_forward, L298N_in1, L298N_in2, L298N_enA, right_wheel_vel_filtered_last);
    motorDirectionSwitch(left_wheel_dir, left_wheel_forward, L298N_in3, L298N_in4, L298N_enB, left_wheel_vel_filtered_last);

    // -- Compute PID --
    if(right_wheel_target_vel == 0.0 && r_setpoint < 0.1)
    {
      right_wheel_pwm = 0.0;
    }
    else
    {
      // Computes PWM
      rightPID.Compute();
    }

    if(left_wheel_target_vel == 0.0 && l_setpoint < 0.1)
    {
      left_wheel_pwm = 0.0;
    }
    else
    { 
      // Computes PWM
      leftPID.Compute();
    }

    // -- Apply PWM --
    analogWrite(L298N_enA, right_wheel_pwm);
    analogWrite(L298N_enB, left_wheel_pwm);

    // -- Send Telemetry Data --
    sendTelemetry();

  }

}

// -- HELPER FUNCTIONS --
// Read Serial Data
void readSerial()
{
  static byte idx = 0;
  char chr;
  const char end_marker = '\n';

  while(Serial.available() > 0 && !new_data)
  {
    // -- Read Character from Buffer --
    chr = Serial.read();

    // -- Store valid characters in the Buffer --
    if(chr != end_marker)
    {
      if(idx < MAX_CHARS - 1)
      {
        serial_bytes[idx] = chr;
        idx++;
      }
    }
    else
    {
      // Reset Loop if endline character is detected
      serial_bytes[idx] = '\0';
      idx = 0;
      new_data = true;
    }
  }
}

// Parse the Data Buffer
void parseData()
{
  if(new_data)
  {
    // Buffer: "rp15.20,ln10.10"
    char* cmd_buffer;

    // Separate character array at ','
    cmd_buffer = strtok(serial_bytes, ",");
    if(cmd_buffer != NULL)
    {
      // Parse Motor Commands. Command: "rp15.20"
      parseMotorCmd(cmd_buffer, right_wheel_dir, right_wheel_target_vel);
    }

    // Get the remaining array
    cmd_buffer = strtok(NULL, ",");
    if(cmd_buffer != NULL)
    {
      // Parse Motor Commands. Command: "ln10.10"
      parseMotorCmd(cmd_buffer, left_wheel_dir, left_wheel_target_vel);
    }

  }
  // Reset
  new_data = false;
}

// Parse Motor Commands
void parseMotorCmd(char* cmd, char& dir, double& vel)
{
  // cmd: "rp15.20"
  if(strlen(cmd) > 1)
  {
    dir = cmd[1]; // Sets wheel direction. Eg: Sets right_wheel_dir to 'p' 
    vel = atof(&cmd[2]); // Converts "15.20" to 15.20f
  }
}

// Ramp Generator
void updateRamp(double& current, double& target, double& step)
{
  // current: current wheel velocity, target: target wheel velocity, step: acceleration step (per loop execution)
  if(current < target)
  {
    current = current + step;
    if(current > target)
    {
      current = target;
    }
  }
  else if(current > target)
  {
    current = current - step;
    if(current < target)
    {
      current = target;
    }
  }
}

// Motor Direction Switching
void motorDirectionSwitch(char& target_dir, bool& forward_dir, const byte& pin1, const byte& pin2, const byte& pinEN, double& filter_hist)
{
  if(target_dir == 'p' && !forward_dir)
  {
    // Switching from backward to forward
    // Reset PWM
    analogWrite(pinEN, 0);
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
    forward_dir = true;
    filter_hist = 0.0;
  }
  else if(target_dir == 'n' && forward_dir)
  {
    // Switching from forward to backward
    // Reset PWM
    analogWrite(pinEN, 0);
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
    forward_dir = false;
    filter_hist = 0.0;
  }
}

// Send Telemetry
void sendTelemetry()
{
  // Send feedback to Serial Port buffer
  Serial.print("r");
  Serial.print(right_wheel_vel_signed >=0 ? "p" : "n");
  Serial.print(right_wheel_vel_filtered, 2);
  Serial.print(",l");
  Serial.print(left_wheel_vel_signed >=0 ? "p" : "n");
  Serial.print(left_wheel_vel_filtered, 2);
  Serial.println(",");
}

// ENCODER ISR CALLBACKS
void rightEncoderCallback()
{

  unsigned long now = micros();
  if(now - right_tick_last_micros < 400)
  {
    return;
  }
  bool A = (PIND & (1 << 2));
  bool B = (PIND & (1 << 4));

  right_encoder_count += (A == B) ? 1 : -1;

  right_tick_last_micros = now;
}

void leftEncoderCallback()
{
  unsigned long now = micros();
  if(now - left_tick_last_micros < 400)
  {
    return;
  }

  bool A = (PIND & (1 << 3));
  bool B = (PIND & (1 << 5));

  left_encoder_count += (A == B) ? -1 : 1;

  left_tick_last_micros = now;
}
