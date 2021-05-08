/*
dekatronStep Dek1(1, 0, 12, 13, 11, true, 0);
dekatronStep Dek2(2, 0, 9, 10, 8, true, 0);
dekatronStep Dek3(3, 0, 6, 7, 5, true, 0);
dekatronStep Dek4(4, 0, 3, 4, 2, true, 0);
dekatronStep Dek5(5, 0, 30, 32, 28, true, 0);
dekatronStep Dek6(6, 0, 26, 24, 22, true, 0);
dekatronStep Dek7(7, 0, 25, 23, 27, true, 0);
dekatronStep Dek8(8, 0, 29, 31, 33, true, 0); // fault in hardware
dekatronStep Dek9(9, 0, 35, 39, 37, true, 0);
dekatronStep Dek10(10, 0, 41, 45, 43, true, 0);

//not connected
dekatronStep Dek11(11, 0, 40, 42, 44, true, 0);
dekatronStep Dek12(12, 0, 34, 38, 36, true, 0);
dekatronStep Dek13(13, 0, 69, 68, 67, true, 0);
dekatronStep Dek14(14, 0, 66, 65, 64, true, 0);
dekatronStep Dek15(15, 0, 63, 62, 61, true, 0);

 */
#include <TimerOne.h>

int stepCount = 67;        // step count
int state = 0;        // State - Machine State
int stepState = 0;        // Dekatron Guide count - 0,1, or 2
int LED = 13;         // Test LED
int Guide1 = 6;       // Guide 1 - G1 pin of 2-guide Dekatron
int Guide2 = 7;       // Guide 2 - G2 pin of 2-guide Dekatron
int indexPin = 5;       // indexPin   - index input pin. High when glow at K0
int index = 0;          // K0 indexPin indicator variable
int Tick = false;     // 500uS tick
int idleCounter = 250;       // Idle counter
int ten_mS_counter = 20;       // 10mS counter
int analogInValue = 0;       // ADC value scaled
//int sensorPin = A0;   // Analog input 0
int sensorPin = 0;    // Analog input 0


// setup() runs once, at reset, to initialize system
// initialize hardware and preset variables
void setup() {
	Timer1.initialize(500);
	Timer1.attachInterrupt(timerISR);
	pinMode(Guide1, OUTPUT);
	pinMode(Guide2, OUTPUT);
	pinMode(indexPin, INPUT);
	pinMode(LED, OUTPUT);

	digitalWrite(LED, LOW);
	stepCount = 67;  //    preset 33mS
	state = 0;
	index = 0;
	go_Clockwise();
}


void d_step(int stepState) // Dekatron Step
{
	if (digitalRead(indexPin)) index = true;   // Sample for glow at K0
	switch (stepState) {
	case 0:                             // glow at a main cathode
		digitalWrite(Guide1, LOW);
		digitalWrite(Guide2, LOW);
		break;
	case 1:                             // glow to a Guide 1 cathode
		digitalWrite(Guide1, HIGH);
		digitalWrite(Guide2, LOW);
		break;
	case 2:                             // glow to a Guide 2 cathode
		digitalWrite(Guide1, LOW);
		digitalWrite(Guide2, HIGH);
	}
}

void go_Clockwise()                    // Dekatron Advance - Clockwise
{
	stepState++;
	if (stepState == 3) stepState = 0;
	d_step(stepState);
}

void go_CounterClockwise()                    // Dekatron Reverse - Counter-Clockwise
{
	stepState--;
	if (stepState == -1) stepState = 2;
	d_step(stepState);
}

//
// Idle - States (0 - 3)
//

void slowCW() {             // Dekatron Action Routine 0 - Slow CW
	if (index) {                    //   When glow hits index [K0] cathode, 
		go_CounterClockwise();                    //    step CCW
		index = false;                //    go to state 1
		state = 1;                  //
		stepCount = 67;                  //    preset 33mS
	}
	else {                        // Continue CW if not at index
		stepCount--;                     //
		if (stepCount == 0) {              //
			go_Clockwise();                  //   CW one step every
			stepCount = 67;                //   33mS
		}
	}
}

/*
void coastCCW() {             // Dekatron Action Routine 1 - Coast CCW
	index = false;                  // 
	stepCount--;                       // 
	if (stepCount == 0) {                // After 33mS
		go_CounterClockwise();                    //  step once CCW
		index = false;                //  go to state 2
		state = 2;                  //
		stepCount = 66;                  //
	}
}
*/

void slowCCW() {             // Dekatron Action Routine 2 - Slow CCW
	if (index) {                    //   When glow hits index [K0] cathode, 
		go_Clockwise();                    //    step CW
		index = false;                //    go to state 3
		state = 3;                  //
		stepCount = 67;                  //    preset 33mS
	}
	else {                        // Continue CCW if not at index
		stepCount--;                     //
		if (stepCount == 0) {              //
			go_CounterClockwise();                  //   CCW one step every
			stepCount = 67;                //   33mS
		}
	}
}
/*
void coastCW() {             // Dekatron Action Routine 3 - Coast CW
	index = false;                  // 
	stepCount--;                       // 
	if (stepCount == 0) {                // After 33mS
		go_Clockwise();                    //  step once CCW
		index = false;                //  go to state 0
		state = 0;                  //
		stepCount = 66;                  //
	}
}
*/

void returnToIndexCCW() {             // Dekatron Action Routine 4 - Return 2
	if (index) {                    //   Return to index, CCW
		index = false;                //     finally at index, go to state 5
		state = 5;                  //     set stepCount per ADC for excursion
		stepCount = analogInValue;
		if (stepCount == 0) stepCount = 1;
	}
	else {
		go_CounterClockwise();
	}
}

void gaugeToStepcount() {             // Dekatron Action Routine 5 - Gauge to stepCount CW
	if (stepCount == 0) {                // 
		index = false;                // when stepCount dots out, go to state 4
		state = 4;                  //
	}
	else {                        // go stepCount dots CW
		stepCount--;                     //
		go_Clockwise();                    //
	}
}


void idleTimer() {
	if (analogInValue < 2) {
		//    digitalWrite(LED, LOW);
		if (idleCounter == 0) {
			if (state > 3) {
				state = 0;
				stepCount = 1;
			}
		}
		else idleCounter--;
	}
	else {
		idleCounter = 250;
		//    digitalWrite(LED, HIGH);
		if (state < 4) {
			state = 4;
			stepCount = 1;
		}
	}
}

// Main Loop - State Machine
//
//  States: 0 -> 1 -> 2 -> 3 -> 0  Idle
//          4 -> 5 -> 4            Gauge (when analog above threshold)
//
void loop() {
	if (Tick) {
		Tick = false;
		switch (state) {            // Do action per current state
		case 0:
			slowCW();             // Idle: Slow_CCW
		//	digitalWrite(LED, LOW);
			break;
		case 1:
		//	coastCCW();             //       Coast_CCW
		//	digitalWrite(LED, HIGH);
			break;
		case 2:
			slowCCW();             //       Slow_CW
		//	digitalWrite(LED, LOW);
			break;
		case 3:
		//	coastCW();             //       Coast_CW
		//	digitalWrite(LED, HIGH);
			break;
		case 4:
			returnToIndexCCW();             // Gauge: Return 2
			break;
		case 5:
			gaugeToStepcount();             //        Gauge_CW
		}
		ten_mS_counter--;
		if (ten_mS_counter == 0) {               // Every 10mS: Read ADC
			ten_mS_counter = 20;
			analogInValue = analogRead(sensorPin) / 35;  // 0-5V
		 //   analogInValue = analogRead(sensorPin)/56;  // Microphone Option
			idleTimer();                    // Idle Timer
		}
	}
}

void timerISR() {
	Tick = true;
}