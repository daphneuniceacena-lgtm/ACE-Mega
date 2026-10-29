/*
  ACE Mega - sumobot firmware
  Cytron Maker RP2040 (Arduino-Pico core), 2x BTS7960, 5 SHARP IR, 2 IR line sensors,
  JSumo start module, Bluetooth module on Serial1 (9600 baud).

  EDIT BEFORE UPLOAD: every pin and threshold below is a placeholder.

  ADC WARNING: the RP2040 has only 3 usable analog inputs (GP26, GP27, GP28). The 3 front SHARP
  sensors use them. The 2 side sensors need a different input: an I2C ADC (ADS1115), an analog
  multiplexer (74HC4051), or digital SHARP modules. Until you rewrite readExtraSharp(), the side
  sensors read 0, which means "nothing seen".

  Line sensors are read as DIGITAL here (IR modules with a digital output). If yours are analog
  and wired to ADC pins, change readLine() to analogRead and set LINE_EDGE_THRESHOLD.

  Bluetooth, while waiting for the start signal:
    '0' spin search   '1' charge first   '2' arc right first   'r' print sensor readings
*/

// ---------- Pins (EDIT) ----------
const uint8_t FRONT_ADC_PINS[3] = {26, 27, 28};       // front left, front center, front right
const uint8_t LINE_PINS[2]      = {10, 11};           // left, right (digital)
const uint8_t START_PIN = 12;
const bool    START_ACTIVE_HIGH = true;               // JSumo module: HIGH while the match runs
const uint8_t L_RPWM = 2, L_LPWM = 3;                 // left BTS7960 (swap sides here if reversed)
const uint8_t R_RPWM = 4, R_LPWM = 5;                 // right BTS7960
const uint8_t EN_PIN = 6;                             // drives R_EN and L_EN of both modules

// ---------- Thresholds (EDIT, readings are scaled to 0-1023) ----------
// SHARP_DETECT_THRESHOLD = a / d_detect + b      SHARP_MAX_READING = a / d_close + b
float SHARP_DETECT_THRESHOLD = 200;
float SHARP_MAX_READING      = 600;
int   LINE_EDGE_THRESHOLD    = 0;                     // digital: reading 1 is above 0
const bool EDGE_IS_HIGH      = true;                  // true if the module outputs HIGH over the white border

int strategy = 0;

int readExtraSharp(int i) { return 0; }               // TODO: side sensors (index 0 and 4), see the warning above

int readSharp(int i) {
  if (i >= 1 && i <= 3) return analogRead(FRONT_ADC_PINS[i - 1]);
  return readExtraSharp(i);
}
int readLine(int i) { return digitalRead(LINE_PINS[i]); }

void setBTS(uint8_t rpwm, uint8_t lpwm, int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) { analogWrite(rpwm, speed); analogWrite(lpwm, 0); }
  else            { analogWrite(rpwm, 0);     analogWrite(lpwm, -speed); }
}
void drive(int left, int right) {
  setBTS(L_RPWM, L_LPWM, left);
  setBTS(R_RPWM, R_LPWM, right);
}

// ================= Core sumobot logic (same in Micro, Mega, Nano) =================
// Priority each loop: 1) edge of the dohyo  2) opponent seen  3) opening move / search
// Sensor order in every array: 0 = left side, 1 = front left, 2 = front center,
//                              3 = front right, 4 = right side
// Line sensors: 0 = left, 1 = right

const int N_SHARP = 5;

// Speeds, 0-255
int ATTACK_BASE  = 140;   // speed when the opponent is just inside detection range
int ATTACK_MAX   = 255;   // speed when the opponent is at its closest
int SEARCH_SPEED = 110;
int TURN_SPEED   = 180;
int BACK_SPEED   = 200;
int STEER_BIAS   = 30;    // extra speed difference when a front-side sensor also sees the opponent
const unsigned long EDGE_BACK_MS  = 220;   // back away from the edge
const unsigned long LINE_SPIN_MS  = 180;   // then spin away from it
const unsigned long OPENING_MS    = 400;   // length of the opening move (strategies 1 and 2)

int  lastSeen = 1;        // side the opponent was last seen on: +1 right, -1 left
bool wasRunning = false;
unsigned long matchStart = 0;

void hwInit();
void idleTasks();

void stopMotors() { drive(0, 0); }

bool startActive() { return (digitalRead(START_PIN) == HIGH) == START_ACTIVE_HIGH; }

bool isEdge(int raw) { return EDGE_IS_HIGH ? raw > LINE_EDGE_THRESHOLD : raw < LINE_EDGE_THRESHOLD; }

// Speed grows with the reading, from ATTACK_BASE at the detection threshold to ATTACK_MAX at the
// closest reading. The reading follows r = a/d + b, see docs/sharp_ir_distance.md.
int speedFromReading(int r) {
  float t = (r - SHARP_DETECT_THRESHOLD) / (SHARP_MAX_READING - SHARP_DETECT_THRESHOLD);
  t = constrain(t, 0.0, 1.0);
  return ATTACK_BASE + (int)((ATTACK_MAX - ATTACK_BASE) * t);
}

// Waits ms milliseconds but returns false (and stops) the moment the start module is released.
bool holdFor(unsigned long ms) {
  unsigned long t = millis();
  while (millis() - t < ms) {
    if (!startActive()) { stopMotors(); return false; }
  }
  return true;
}

void printReadings(Print &p) {
  p.print("S:");
  for (int i = 0; i < N_SHARP; i++) { p.print(readSharp(i)); p.print(' '); }
  p.print(" L:"); p.print(readLine(0)); p.print(' '); p.println(readLine(1));
}

void runMatchStep() {
  // 1. Edge: back away, then spin away from the side that saw the edge
  bool eL = isEdge(readLine(0)), eR = isEdge(readLine(1));
  if (eL || eR) {
    drive(-BACK_SPEED, -BACK_SPEED);
    if (!holdFor(EDGE_BACK_MS)) return;
    if (eR && !eL) drive(-TURN_SPEED, TURN_SPEED);      // edge on the right: spin left
    else           drive(TURN_SPEED, -TURN_SPEED);      // edge on the left or both: spin right
    holdFor(LINE_SPIN_MS);
    return;
  }

  // 2. Opponent
  int r[N_SHARP];
  bool seen[N_SHARP];
  for (int i = 0; i < N_SHARP; i++) {
    r[i] = readSharp(i);
    seen[i] = r[i] > SHARP_DETECT_THRESHOLD;
  }

  if (seen[2]) {                                         // straight ahead: charge
    int s = speedFromReading(r[2]);
    int steer = (seen[3] ? 1 : 0) - (seen[1] ? 1 : 0);   // +1 means the opponent is a bit to the right
    if (steer != 0) lastSeen = steer;
    drive(s + STEER_BIAS * steer, s - STEER_BIAS * steer);
    return;
  }
  if (seen[1]) { int s = speedFromReading(r[1]); lastSeen = -1; drive(s / 3, s); return; }   // arc left
  if (seen[3]) { int s = speedFromReading(r[3]); lastSeen =  1; drive(s, s / 3); return; }   // arc right
  if (seen[0]) { lastSeen = -1; drive(-TURN_SPEED, TURN_SPEED); return; }                    // pivot left
  if (seen[4]) { lastSeen =  1; drive(TURN_SPEED, -TURN_SPEED); return; }                    // pivot right

  // 3. Nothing seen: opening move for the first moments, then search
  unsigned long t = millis() - matchStart;
  if (strategy == 1 && t < OPENING_MS) { drive(ATTACK_MAX, ATTACK_MAX); return; }    // charge straight
  if (strategy == 2 && t < OPENING_MS) { drive(ATTACK_MAX, ATTACK_BASE); return; }   // arc right to flank
  drive(lastSeen * SEARCH_SPEED, -lastSeen * SEARCH_SPEED);                          // spin toward last seen side
}

void setup() {
  hwInit();
  pinMode(START_PIN, INPUT);
  stopMotors();
}

void loop() {
  if (!startActive()) {                    // waiting, or the start module killed the match
    stopMotors();
    wasRunning = false;
    idleTasks();
    return;
  }
  if (!wasRunning) {                       // match just started (the module counts down its own 5 s)
    wasRunning = true;
    matchStart = millis();
    lastSeen = 1;
  }
  runMatchStep();
}

// ---------- Hardware init and idle tasks ----------
void hwInit() {
  analogReadResolution(10);                // keep readings on the same 0-1023 scale as the Nano robots
  analogWriteFreq(20000);                  // quiet, smooth PWM for the BTS7960
  pinMode(EN_PIN, OUTPUT); digitalWrite(EN_PIN, HIGH);
  pinMode(LINE_PINS[0], INPUT); pinMode(LINE_PINS[1], INPUT);
  Serial1.begin(9600);
}

void idleTasks() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c >= '0' && c <= '2') { strategy = c - '0'; Serial1.print("strategy "); Serial1.println(strategy); }
    else if (c == 'r') printReadings(Serial1);
  }
}
