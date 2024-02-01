
# include <SoftwareSerial.h>


//12v Input

SoftwareSerial BT(11, 10); //HC-05 PIN, RX10,TX11

// constant
const int iend = 60;    //end command
const int speed_max = 100;  //speed max
const int speed_min = 40;   //speed_min
const int joystick_max = 50;
const int wheel1speedPIN = 3
                           , wheel2speedPIN = 5
                               , wheel3speedPIN = 6
                                   , wheel4speedPIN = 9; //controll motor speed
const int data = 2, big = 4, push = 7;

//variable
int cmmd[10];   //command
int insize;   //detect Bluetooth available
float vx, vy, d ;   //the data from phone, vx vx distance
int i = 0;

float v2, v1, v3, v4; // speed of four wheel
int arnd = 0; // 0: do not turn around;
//1: turn  clockwise;
//-1: turn counterclockwise

int three_to_eight[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //3 to 8 data

float Move_vy;
float Move_vx;

//function
void Stop();
int convert(int n);   //convert the value arduino receive
void get_value();   // get the value from phone
void translate();   //translate the value if the speed is lower than limit

void wheel(int wheel_num, int wheel_direction , int wheelspeed);
void  putsomething(int put_way);
void HC595();   //controll all

void auto_wheel(float v1, float v2, float v3, float v4);    //give four wheels speed
void mecanumRun(float xSpeed, float ySpeed, float aSpeed);    //control mecanum


void Move_line(float v, float theta, float t);  //moving, v, direction: theta(360'), time: sec
void Move_Square(float v, float r);   //draw a square with speeed : v, side length:r
void Move_Circle(float v, float r);    //draw a circle with speeed : v, side length:r





void setup() {

  Serial.begin(9600);
  BT.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(big, OUTPUT);
  pinMode(push, OUTPUT);
  pinMode(wheel1speedPIN, OUTPUT);
  pinMode(wheel2speedPIN, OUTPUT);
  pinMode(wheel3speedPIN, OUTPUT);
  pinMode(wheel4speedPIN, OUTPUT);

  //test

  delay(3000);
  /*
    mecanumRun(0, 0, 150);
    delay(2000);
    mecanumRun(0, 0, 0);
  */

  //Move_line(100, 0, 1);   //test1
  //Move_Square(100, 15);   //test2
  Move_Circle(130, 1);


}


void loop() {

  //Move_line(50, 90, 1);   //test1


  /*
    get_value();
    wheel(1, (v1 / abs(v1)), abs(v1));
    wheel(2, (v2 / abs(v2)), abs(v2));
    wheel(3, (v3 / abs(v3)), abs(v3));
    wheel(4, (v4 / abs(v4)), abs(v4));
  */

}


//=====================

float theta_angle(float a)
{
  return ( a = a * 2 * M_PI / 360 );
}

void mecanumRun(float xSpeed, float ySpeed, float aSpeed)
{
  float k = sin(theta_angle(45)) ;  //k=sin45'
  float n = 1 ; //n=L*cos(α/2 +45')
  xSpeed = xSpeed / k;
  ySpeed = ySpeed / k;
  aSpeed = aSpeed / n;

  float speed1 = ( xSpeed + ySpeed - aSpeed) / 4;
  float speed2 = (-xSpeed + ySpeed + aSpeed) / 4;
  float speed3 = (-xSpeed + ySpeed - aSpeed) / 4;
  float speed4 = ( xSpeed + ySpeed + aSpeed) / 4;

  float max = speed1;
  if (max < speed2)   max = speed2;
  if (max < speed3)   max = speed3;
  if (max < speed4)   max = speed4;

  if (max > speed_max)
  {
    speed1 = speed1 / max * speed_max;
    speed2 = speed2 / max * speed_max;
    speed3 = speed3 / max * speed_max;
    speed4 = speed4 / max * speed_max;
  }
  auto_wheel(speed1, speed2, speed3, speed4);
}


void Move_Circle(float v, float r) {

  float tn = 46;
  float xSpeed = v;   //v=rw, w=v/r
  float aSpeed = v / r;
  float ySpeed = 0;
  float T = tn * 2 * M_PI / aSpeed ; // T=2pi/w

  mecanumRun(xSpeed, ySpeed, aSpeed);

  delay(T * 1000);
  auto_wheel(0, 0, 0, 0);
}

void Move_Square(float v, float r) {
  const float k = 5;  // proportion constant
  float t = k * r / v;

  Move_line(v, 0, t);  //right
  delay(500);
  Move_line(v, -90, t);  //back
  delay(500);
  Move_line(v, 180, t);  //left
  delay(500);
  Move_line(v, 90, t);   //forward
}

void Move_line(float v, float theta, float t) {
  /*
    Serial.println(theta);
    Serial.println(Move_vx);
    Serial.println(Move_vy);
  */
  theta = theta_angle(theta);

  Move_vx = v * cos(theta);
  Move_vy = v * sin(theta);
  /*
    Serial.println(theta);
    Serial.println(Move_vx);
    Serial.println(Move_vy);
  */

  mecanumRun(Move_vx, Move_vy, 0 );

  delay(t * 1000);
  auto_wheel(0, 0, 0, 0); //stop
}


void auto_wheel(float v1, float v2, float v3, float v4) {
  wheel(1, (v1 / abs(v1)), abs(v1));
  wheel(2, (v2 / abs(v2)), abs(v2));
  wheel(3, (v3 / abs(v3)), abs(v3));
  wheel(4, (v4 / abs(v4)), abs(v4));
}


void Stop() {
  vx = vy = arnd = 0;
}

int convert(int n)
{
  if (n > 50)
    return (n - 256);
  return (n);
}

void get_value()
{

  // get the bluetooth texts
  while (((insize = (BT.available())) > 0 ) && (cmmd[i] = (BT.read())) != iend)
  {
    i ++;
  }

  //One command has done
  if ((cmmd[i] ) == iend)
  {
    BT.read();  // clear 0 after list
    i = 0;

    // discriminate command
    switch (cmmd[0])    //first letter is S or J or T
    {
      case 'S':
        Stop();
        break;
      case 'J':   //J
        if (cmmd[1] == 'X')
        {
          vx = convert(cmmd[2]);
        }
        if (cmmd[3] == 'Y')
        {
          vy = convert(cmmd[4]);
        }
        if (cmmd[5] == 'D')
        {
          d = cmmd[6];
        }

      case 'T':    //T
        switch (cmmd[1])
        {
          case 'R':  //TR
            arnd = 1;


            break;
          case 'L':  //TL
            arnd = -1;
            break;
        }
        break;
    }
  }

  // change the vlue for machine
  if (arnd == 0) // don't need turn around
  {
    v1 = v3  = speed_max * ((vx + vy) / 2 / joystick_max );
    v2 = v4 = speed_max * ((vy - vx) / 2 / joystick_max );
    translate();

  }

  else if (arnd == 1)  //clockwise
  {
    v1  = v4 = speed_max / 2;
    v3 =  v2 = -speed_max / 2;
    // Serial.println("TR");

  }

  else if (arnd == -1) //counterclockwise
  {
    v1 = v4 = -speed_max / 2;
    v3 = v2 = speed_max / 2;
    // Serial.println("TL");

  }

}

void translate() {
  if (abs(v1) < speed_min && v1 != 0) {

    v2 = v4 = speed_min * (v2 / v1) * (abs(v2 * v1) / int(v2 * v1)) * (abs(v2) / int(v2));
    //Ratio    //offset positive or negative      //return p. or n.
    v1 = v3 = speed_min * (abs(v1) / int(v1));
    //return p. or n.
  }

  if (abs(v2) < speed_min && v2 != 0) {
    v1 = v3 = speed_min * (v1 / v2) * (abs(v2 * v1) / int(v2 * v1)) * (abs(v1) / int(v1));
    //Ratio    //offset positive or negative      //return p. or n.
    v2 = v4 = speed_min * (abs(v2) / int(v2));
    //return p. or n.
  }
  v1 = v3 = int(v1);
  v2 = v4 = int(v2);
}

/*
  IN1 IN2 Status
  1    0  +
  0    1  -
  0    0  Stop
*/

//12v Input



void wheel(int wheel_num, int wheel_dir , int wheelspeed) {

  //Motor 1
  if (wheel_num == 1) {
    if (wheel_dir == 1) {
      three_to_eight[2] = 1;
      three_to_eight[3] = 0;
      analogWrite(wheel2speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == -1) {
      three_to_eight[2] = 0;
      three_to_eight[3] = 1;
      analogWrite(wheel2speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == 0) {
      three_to_eight[2] = 0;
      three_to_eight[3] = 0;
      analogWrite(wheel1speedPIN, 0);
    }
  }

  //Motor 2
  if (wheel_num == 2) {
    if (wheel_dir == 1) {
      three_to_eight[0] = 1;
      three_to_eight[1] = 0;
      analogWrite(wheel1speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == -1) {
      three_to_eight[0] = 0;
      three_to_eight[1] = 1;
      analogWrite(wheel1speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == 0) {
      three_to_eight[0] = 0;
      three_to_eight[1] = 0;
      analogWrite(wheel1speedPIN, 0);
    }
  }

  //Motor 3
  if (wheel_num == 3) {
    if (wheel_dir == 1) {
      three_to_eight[4] = 1;
      three_to_eight[5] = 0;
      analogWrite(wheel3speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == -1) {
      three_to_eight[4] = 0;
      three_to_eight[5] = 1;
      analogWrite(wheel3speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == 0) {
      three_to_eight[4] = 0;
      three_to_eight[5] = 0;
      analogWrite(wheel1speedPIN, 0);
    }
  }
  //Motor 4
  if (wheel_num == 4) {
    if (wheel_dir == 1) {
      three_to_eight[6] = 1;
      three_to_eight[7] = 0;
      analogWrite(wheel4speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == -1) {
      three_to_eight[6] = 0;
      three_to_eight[7] = 1;
      analogWrite(wheel4speedPIN, map(wheelspeed, 0, 100, 0, 255));
    }
    if (wheel_dir == 0) {
      three_to_eight[6] = 0;
      three_to_eight[7] = 0;
      analogWrite(wheel1speedPIN, 0);
    }
  }
  HC595();
  // Serial.println(wheel_dir);
}

void HC595() {
  for (int HC595_i = 7 ; HC595_i > -1; HC595_i--) {
    putsomething(three_to_eight[HC595_i]);
  }
  digitalWrite(big, 1);
  digitalWrite(big, 0);
}
void  putsomething(int wheel_status) {
  digitalWrite(data, wheel_status);
  digitalWrite(push, 1);
  digitalWrite(push, 0);
}
