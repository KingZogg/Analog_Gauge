/*
  Dekatron Analog Gauge 2 Example - with Timer IRQ

  modified April 7, 2017
  by Michael Moorrees
 */
#include <TimerOne.h>

int Scnt = 67;        // step count
int state = 0;        // State - Machine State
int dk_ct = 0;        // Dekatron Guide count - 0,1, or 2
int LED = 13;         // Test LED
int Guide1 = 28;       // Guide 1 - G1 pin of 2-guide Dekatron
int Guide2 = 26;       // Guide 2 - G2 pin of 2-guide Dekatron
int Index = 24;       // Index   - NDX input pin. High when glow at K0
int Ndx = 0;          // K0 index indicator variable
int Tick = false;     // 500uS tick
int Icnt = 250;       // Idle counter
int ct_10 = 20;       // 10mS counter
int in_val = 0;       // ADC value scaled
//int sensorPin = A0;   // Analog input 0
int sensorPin = 0;    // Analog input 0


// setup() runs once, at reset, to initialize system
// initialize hardware and preset variables
void setup() {
	Timer1.initialize(500);
	Timer1.attachInterrupt(timerISR);
	pinMode(Guide1, OUTPUT);
	pinMode(Guide2, OUTPUT);
	pinMode(Index, INPUT);
	pinMode(LED, OUTPUT);

	digitalWrite(LED, LOW);
	Scnt = 67;
	state = 0;
	Ndx = 0;
	D_adv();
}


void d_step(int dk_bt) // Dekatron Step
{
	if (digitalRead(Index)) Ndx = true;   // Sample for glow at K0
	switch (dk_bt) {
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

void D_adv()                    // Dekatron Advance - Clockwise
{
	dk_ct++;
	if (dk_ct == 3) dk_ct = 0;
	d_step(dk_ct);
}

void D_rev()                    // Dekatron Reverse - Counter-Clockwise
{
	dk_ct--;
	if (dk_ct == -1) dk_ct = 2;
	d_step(dk_ct);
}

//
// Idle - States (0 - 3)
//

void dk_action0() {             // Dekatron Action Routine 0 - Slow CW
	if (Ndx) {                    //   When glow hits Ndx [K0] cathode, 
		D_rev();                    //    step CCW
		Ndx = false;                //    go to state 1
		state = 1;                  //
		Scnt = 67;                  //    preset 33mS
	}
	else {                        // Continue CW if not at Ndx
		Scnt--;                     //
		if (Scnt == 0) {              //
			D_adv();                  //   CW one step every
			Scnt = 67;                //   33mS
		}
	}
}

void dk_action1() {             // Dekatron Action Routine 1 - Coast CCW
	Ndx = false;                  // 
	Scnt--;                       // 
	if (Scnt == 0) {                // After 33mS
		D_rev();                    //  step once CCW
		Ndx = false;                //  go to state 2
		state = 2;                  //
		Scnt = 66;                  //
	}
}

void dk_action2() {             // Dekatron Action Routine 2 - Slow CCW
	if (Ndx) {                    //   When glow hits Ndx [K0] cathode, 
		D_adv();                    //    step CW
		Ndx = false;                //    go to state 3
		state = 3;                  //
		Scnt = 67;                  //    preset 33mS
	}
	else {                        // Continue CCW if not at Ndx
		Scnt--;                     //
		if (Scnt == 0) {              //
			D_rev();                  //   CCW one step every
			Scnt = 67;                //   33mS
		}
	}
}

void dk_action3() {             // Dekatron Action Routine 3 - Coast CW
	Ndx = false;                  // 
	Scnt--;                       // 
	if (Scnt == 0) {                // After 33mS
		D_adv();                    //  step once CCW
		Ndx = false;                //  go to state 0
		state = 0;                  //
		Scnt = 66;                  //
	}
}

void dk_action4() {             // Dekatron Action Routine 4 - Return 2
	if (Ndx) {                    //   Return to Ndx, CCW
		Ndx = false;                //     finally at Ndx, go to state 5
		state = 5;                  //     set Scnt per ADC for excursion
		Scnt = in_val;
		if (Scnt == 0) Scnt = 1;
	}
	else {
		D_rev();
	}
}

void dk_action5() {             // Dekatron Action Routine 5 - Gauge to Scnt CW
	if (Scnt == 0) {                // 
		Ndx = false;                // when Scnt dots out, go to state 4
		state = 4;                  //
	}
	else {                        // go Scnt dots CW
		Scnt--;                     //
		D_adv();                    //
	}
}


void I_tmr() {
	if (in_val < 2) {
		//    digitalWrite(LED, LOW);
		if (Icnt == 0) {
			if (state > 3) {
				state = 0;
				Scnt = 1;
			}
		}
		else Icnt--;
	}
	else {
		Icnt = 250;
		//    digitalWrite(LED, HIGH);
		if (state < 4) {
			state = 4;
			Scnt = 1;
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
			dk_action0();             // Idle: Slow_CCW
			digitalWrite(LED, LOW);
			break;
		case 1:
			dk_action1();             //       Coast_CCW
			digitalWrite(LED, HIGH);
			break;
		case 2:
			dk_action2();             //       Slow_CW
			digitalWrite(LED, LOW);
			break;
		case 3:
			dk_action3();             //       Coast_CW
			digitalWrite(LED, HIGH);
			break;
		case 4:
			dk_action4();             // Gauge: Return 2
			break;
		case 5:
			dk_action5();             //        Gauge_CW
		}
		ct_10--;
		if (ct_10 == 0) {               // Every 10mS: Read ADC
			ct_10 = 20;
			in_val = analogRead(sensorPin) / 35;  // 0-5V
		 //   in_val = analogRead(sensorPin)/56;  // Microphone Option
			I_tmr();                    // Idle Timer
		}
	}
}

void timerISR() {
	Tick = true;
}