//vito game machine. nvf 2k23

#include <Wire.h>
#include <Adafruit_SH110X.h>


#define OLED_RESET 16
#define WHITE 1
#define BLACK 0

Adafruit_SH1106G display(128, 64, &Wire, OLED_RESET);


// 'ufo', 43x15px
const unsigned char epd_bitmap_ufo [] PROGMEM = {
  0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x01, 0x24, 0x90, 
  0x00, 0x00, 0x00, 0x03, 0x24, 0x98, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x07, 
  0xff, 0xfc, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 
  0x00, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x0f, 0xff, 0xff, 0xff, 
  0xfe, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xe0, 0x07, 0x80, 0x7f, 0x00, 0x7c, 0x00
};

//Pin Definitions

int leftBumperPin = A3;
int rightBumperPin = A2;
int button1Pin = 5;
int button2Pin = 4;
float gameSpeed = 3;
int buzzerPin = 6;






class Menu {            //Menu with rounded corners. 
  public:               
    int* button1Pin;
    int* button2Pin;
    int* leftBumperPin;
    int currentSelection;
    int lastcurrentSelection;
    String menuItems[4];        //The size of this array defines the size of the menu. Make sure it matches numItems below!
    int numItems;
    int button1State;

    Menu(int inbutton1Pin, int inbutton2Pin, int inleftBumperPin) {
      button1Pin = inbutton1Pin;
      button2Pin = inbutton2Pin;
      leftBumperPin = inleftBumperPin;
      pinMode(button1Pin, INPUT_PULLUP);
      pinMode(button2Pin, INPUT_PULLUP);
      menuItems[0] = F("Pong Game");
      menuItems[1] = F("Song Maker");
      menuItems[2] = F("Etch a Sketch");
      menuItems[3] = F("Who am I?");
      numItems = 4;
      currentSelection = 0;
      lastcurrentSelection = 99;
      button1State = 1;
      
      button1State = digitalRead(button1Pin);
      display.setTextSize(1);
    }
    int show(){                                                                   //show(): display menu. Only rewrites the display on selection changes, which frees time for checking button input
      int rawReading = analogRead(leftBumperPin);
      if (rawReading < 10) {                      //Edge detection to fix weird behavior at end of potentiometer travel
        rawReading = 0;
      }
      else if (rawReading > 1000) {
        rawReading = 1023;
      }
      rawReading = rawReading;             //Reverse reading so that scrolling feels natural 
      currentSelection =  map(rawReading, 0, 1023, 0, numItems-1); 
      int itemHeight = int(display.height()/numItems); 
         
      int newbutton1State = digitalRead(button1Pin);   
      if ((newbutton1State == LOW)&&(button1State ==0)) {   //New button press
        button1State=1;
        lastcurrentSelection=99;
        return(currentSelection);
      }
      else if (newbutton1State == HIGH){
        button1State = 0;
      }

     if (currentSelection != lastcurrentSelection){                //Update and draw Menu

      
      for (int i=0; i<numItems; i++){                                                                     //Generate menu items and selective highlighting 
        display.fillRoundRect(0, (i)*itemHeight, int(display.width()), itemHeight, 3, WHITE);
        display.setCursor(int(float(display.width()/2)-3*menuItems[i].length()),(i+0.4)*itemHeight );     //Centered horizontally and vertically on the menu item 
        if (i ==  currentSelection){   
          display.setTextColor(BLACK);
        }
        else {                                                                                            //Non-highlighted item
          display.fillRoundRect(1, (i)*itemHeight + 1, int(display.width())-2, itemHeight-2,3, BLACK);
          display.setTextColor(WHITE); 
        }
        display.println(menuItems[i]);                                                                  // Label
      } 
     display.display();
     lastcurrentSelection = currentSelection;
     }
     return(99);      //Default return for if no selection was made
    }
};

class Bumper{ //Bumper class, with basic motion used for collisions with ball
  public:
    float x;
    float y;
    float velx;
    float vely;
    int lastx;
    int lastY;
    int height;
    int width;

    Bumper() {
      x = 0;
      y = 0;
      velx = 0; 
      vely = 0;
      height = 20;
      width = 3;
    }

    void draw(){
      display.fillRect(x, y, width,height, WHITE);

    }
    void clr(){
      display.fillRect(x, y, width,height, BLACK);
    }

    bool updatePosition(){
      x = x + velx;
      y = y + vely;
    }

    bool newupdatePosition(){
      float x1 = x;
      this->updatePosition();
      float x2 = x;
      display.fillRect(x2, y, x1-x2, height, WHITE);
      display.fillRect(x2 + width, y, x1-x2+1, height, BLACK);
      
    }
};

class Sprite{  // Same as bumper, but draw a bitmap
  public:
    float x;
    float y; 
    float velx;
    float vely;
    float accelx;
    float accely;
    Sprite(){
      velx = 0;
      vely = 0;
      accelx = 0;
      accely = 0;
    }
    void updatePosition(){
      x = x + velx;
      velx = velx + accelx;

      y = y + vely;
      vely = vely + accely;
    }
    void draw(){
      display.drawBitmap(x, y, epd_bitmap_ufo, 43, 15, WHITE);
    }
    void clr(){
      display.drawBitmap(x, y, epd_bitmap_ufo, 43, 15, BLACK);
    }
};

class Ball{ //Ball, with basic motion for interaction with bumper
    public:
      float x;
      float y;
      float vely;
      float velx;
      int radius;
      Ball() {
      velx = 0;
      vely = 0;
      }
      bool updatePosition(){
          x = x + velx;
          y = y + vely;
         
      }
      void draw(){
        display.fillCircle(x, y, radius, WHITE);

        }
      void clr(){
        display.fillCircle(x, y, radius, BLACK);
        }
      };


class PongGame {// Pong Game
  public:
    Ball ball;
    Bumper leftBumper;
    Bumper rightBumper;
    int* button1Pin;
    int* leftBumperPin;
    int* rightBumperPin;
    int gameSpeed;
    int button1State;
    int leftPoints;
    int rightPoints;


    void resetGame(){                               //Set position and velocity of the ball
      ball.x = 64;
      ball.y = 32;
      ball.radius = 3;
      ball.velx = random(gameSpeed-3,gameSpeed);    //randomize direction, but make total velocity dependent on gameSpeed.
      bool polarity = random(-1,2);
      ball.vely = gameSpeed-ball.velx;
      ball.velx = pow(-1, polarity)*ball.velx;
    }

    
    PongGame(int inleftBumperPin, int inrightBumperPin,int inbutton1Pin, int ingameSpeed){
      gameSpeed = ingameSpeed;
      rightBumper.x = 124; 
      this->leftBumperPin = inleftBumperPin;
      this->rightBumperPin = inrightBumperPin;
      this->button1Pin = inbutton1Pin;
      this->resetGame();
      this->button1State = 1;
    }

    bool playGame(){            //Returns when game is exited
      leftBumper.y = display.height() - leftBumper.height - map(analogRead(leftBumperPin), 0, 1023, 0, display.height()-leftBumper.height);
      rightBumper.y = map(1023 - analogRead(rightBumperPin), 0, 1023, 0, display.height()-rightBumper.height);        
      leftBumper.draw();
      rightBumper.draw();
      ball.draw();
      display.display();
      delay(1000);          // Allows for a moment before the game starts
      while(1){
        ball.clr();
        ball.updatePosition();

        display.setTextColor(WHITE);        //Update scores
        display.setCursor(0,0);
        display.println(rightPoints);
        display.setCursor(120, 00);
        display.println(leftPoints);


        int newbutton1State = digitalRead(this->button1Pin);
        if ((newbutton1State == LOW)&&(this->button1State == 0)) { 
            button1State = 1;
            delay(10);
            return(0);
        }
        else if (newbutton1State == HIGH){
          this->button1State = 0;
        }
      
      if(!this->updateCollisions()){
        //Entry point for when ball leaves screen. Currently unused.

        }
        ball.draw();                    //Only redraw ball after both position-changing functions (updateCollisions, updatePosition) are called
        
        leftBumper.clr();
        rightBumper.clr();
        leftBumper.lastY = leftBumper.y;
        rightBumper.lastY = rightBumper.y;
        leftBumper.y = display.height() - leftBumper.height - map(analogRead(leftBumperPin), 0, 1023, 0, display.height()-leftBumper.height);
        rightBumper.y = map(1023 - analogRead(rightBumperPin), 0, 1023, 0, display.height()-rightBumper.height);        
        leftBumper.draw();
        rightBumper.draw();
        display.display();
      }     
    }
    bool updateCollisions(){
        if((ball.y + ball.radius >= display.height())||(ball.y - ball.radius <= 0)){         //Ceiling or floor contact
          ball.vely = -1*ball.vely;
          ball.y = ball.y+ball.vely;

          }
        else if (ball.x + ball.radius >= display.width() - rightBumper.width){               //right bumper contact
          if ((ball.y + (0.5*ball.radius) >= rightBumper.y)&&( ball.y - (0.5*ball.radius)<= (rightBumper.y + rightBumper.height))){
          tone(buzzerPin, 1000, 100);
          ball.velx = -1.08*ball.velx;
          ball.x =  display.width() - leftBumper.width - ball.radius;
          int bumperChange = rightBumper.y-rightBumper.lastY;
          if ((abs(bumperChange))> 1){
            ball.vely = ball.vely + ((0.2 + ball.vely*0.3)*(bumperChange/abs(bumperChange)));
          }
          
          }  
        }
        else if (ball.x - ball.radius <= leftBumper.x){                                     //left bumper contact
          if ((ball.y + (0.5*ball.radius) >= leftBumper.y) &&(ball.y - (0.5*ball.radius) <= (leftBumper.y + leftBumper.height))){
            tone(buzzerPin, 1000, 100);
            ball.velx = -1.08*ball.velx;
            ball.x=ball.radius+leftBumper.width;
            int bumperChange = leftBumper.y-leftBumper.lastY;
            if ((abs(bumperChange))> 1){
            ball.vely = ball.vely + ((0.2 + ball.vely*.3)*(bumperChange/abs(bumperChange)));
          }
              
          }
        }
        
        if ((ball.x - ball.radius >= display.width())||(ball.x + ball.radius <=0)){         //ball leaves screen. only 0 return point. 
          
          display.setTextColor(BLACK);
          display.setCursor(0,0);
          display.println(rightPoints);
          display.setCursor(120, 00);
          display.println(leftPoints);
          
          if (ball.x > 32) {
            this->rightPoints++;
          }
          else {
            this->leftPoints++;
          }

          
          this->resetGame();
          ball.draw();

          display.display();
          delay(1000);       
          return(0);
        }
      return(1);
    }   
};




//FlyGame commented out for memory purposes :-(
/*
class FlyGame {         // Flappy bird like-game. Bumpers appear and you gotta dodge em.
  public:
    Ball ball;
    Bumper bumpers[5];
    int gameStart;
    int* button1Pin;
    int* leftBumperPin;
    int* rightBumperPin;
    float gameSpeed;
    int button1State;
    int score;
    int lastUpdate;
    void resetGame(){
      ball.x = 32;
      ball.y = 10;
      ball.radius = 3;
      score = 0;

      bumpers[0].x = 100;
      bumpers[0].y = 0;
      bumpers[0].height = 40;
      bumpers[0].velx = -gameSpeed;
      
      bumpers[1].x = display.height()- 20;
      bumpers[1].velx = -gameSpeed;
      
      bumpers[2].x = 40;
      bumpers[2].height = 13;
      bumpers[2].y = 0;
      bumpers[2].velx = -gameSpeed;
      
      bumpers[3].x = 150;
      bumpers[3].y = display.height() - 16;
      bumpers[3].height = 16;
      bumpers[3].velx = -gameSpeed;

      bumpers[4].x = 110;
      bumpers[4].y = 0;
      bumpers[4].velx = -gameSpeed;
    }
    FlyGame(int inleftBumperPin, int inrightBumperPin,int inbutton1Pin, float ingameSpeed){
      gameSpeed = ingameSpeed;
      this->leftBumperPin = inleftBumperPin;
      this->rightBumperPin = inrightBumperPin;
      this->button1Pin = inbutton1Pin;
      this->button1State = 1;  
      this->resetGame();    
    }

    bool playGame(){
      lastUpdate = 0;
      while(1){


       display.setCursor(109, 0);
       display.setTextColor(WHITE);
       display.print(score);




        
        ball.clr();
        ball.y = display.height() - ball.radius - map(analogRead(leftBumperPin), 0, 1023, 0, display.height()-ball.radius);

        
        bumpers[0].clr();
        bumpers[1].clr();      
        bumpers[2].clr();
        bumpers[3].clr();
        bumpers[4].clr();

        bumpers[0].updatePosition();
        bumpers[1].updatePosition();      
        bumpers[2].updatePosition();
        bumpers[3].updatePosition();
        bumpers[4].updatePosition();

        bumpers[0].draw();
        bumpers[1].draw();      
        bumpers[2].draw();
        bumpers[3].draw();
        bumpers[4].draw();

                
      int newbutton1State = digitalRead(this->button1Pin);
      if ((newbutton1State == LOW)&&(button1State ==0)) {       //Button pressed: exit game
          this->button1State = 1;
          delay(10);
          return(0);
      }
      else if (newbutton1State == HIGH){
        this->button1State = 0;
      }
      
      if(!this->updateCollisions()){
                                                  //TODO Add here: triggers for life counting or something
         return(0);
      }
        ball.draw();
        display.display();
      }     
    }
    bool updateCollisions(){

      for (int i = 0; i < 5; i++){                 //Bumper Collisions
        if ((ball.x + ball.radius >= bumpers[i].x) && (ball.x - ball.radius < bumpers[i].x + bumpers[i].width) && (ball.y >= bumpers[i].y ) && (ball.y <= bumpers[i].y + bumpers[i].height)) { //ball/bumper
          ball.x = bumpers[i].x - gameSpeed -1;
        }
        if (bumpers[i].x + bumpers[i].width < 0){                                   //Bumper off screen. Move back and randomize 
          float randomFactor = random(0, 0.1*display.width());
          float randomFactor2 = random(0,20);
          int randBool = ((int)floor(random(0,100))%2);
          bumpers[i].height = int(14 + randomFactor2);
          if (randBool == 1){
            bumpers[i].y = 0;
          }
          else {
            bumpers[i].y= display.height() - bumpers[i].height;
          }

          bumpers[i].x = int(float((0.9*display.width()) + randomFactor));         //Randomize position and size by up to 4%.
          
          //bumpers[i].y = random(2, display.height() - bumpers[i].height +1);
           display.setCursor(109, 0);               
           display.setTextColor(BLACK);
           display.println(score);
          score++;

          
          for (int j = 0; j < 5; j++) {
              bumpers[j].velx = -gameSpeed*(1 + (0.02*score));
          }

          
        }
      }
        
        if ((ball.x + ball.radius <=0)){          //ball leaves screen. only 0 return point. 
          ball.clr();
  
          display.clearDisplay();                 //Score Display (final score)
          display.setCursor(0, 32);
          display.setTextSize(2);
          display.print(F("score: "));
          display.print(score);
          display.setTextSize(1);
          display.display();
          
          delay(500);
          this->resetGame();
          return(0);
        }
      return(1);
    }


    
};
*/

class SongPlayer {
  public:
    int leftBumperPin;
    int rightBumperPin;
    int button1Pin;
    int button1State;
    int button2Pin;
    int button2State;
    int playtime;
    int lastPitch;
    int freq;
    int vis1height;
    int vis2rad;
    SongPlayer (int inleftBumperPin, int inrightBumperPin, int inbutton1Pin, int inbutton2Pin){
        this->leftBumperPin = inleftBumperPin;
        this->rightBumperPin = inrightBumperPin;
        this->button1Pin = inbutton1Pin;
        this->button2Pin = inbutton2Pin;        
    }
    void drawVis(){

      display.drawRect(64 - this->vis1height, 32 - (this->vis1height/2), this->vis1height*2, this->vis1height, WHITE);
      display.drawCircle(64, 32, 31 - this->vis2rad , WHITE);

    }
    void clearVis(){
      display.drawRect(64 - this->vis1height, 32 - (this->vis1height/2), this->vis1height*2, this->vis1height,  BLACK);
      //display.drawLine(64 - this ->vis1height, 32 - (this->vis1height/2), 64, 32, WHITE);
      display.drawCircle(64, 32, 31 - this->vis2rad, BLACK);
    }

    bool playGame(){
      int notes[14] = {440, 494, 523, 587, 659, 698, 784, 880, 988, 1047, 1175, 1319, 1568, 1760};
      display.clearDisplay();
      display.display();
      while(1){         //Process exit button events
        int newbutton1State = digitalRead(this->button1Pin);
        if ((newbutton1State == LOW)&&(this->button1State ==0)) { 
          this->button1State = 1;
          delay(10);
          display.clearDisplay();
          return(0);
      }
         else if (newbutton1State == HIGH){
          this->button1State = 0;
      }
      this->lastPitch = this->freq;
      int noteNumber = map(1024 - analogRead(this->leftBumperPin), 0, 1023, 0, 13);
      
      this->freq = notes[noteNumber];
      playtime = map(1024-analogRead(this->rightBumperPin), 0, 1023, 50, 200);
      int speedNumber = map(playtime, 50, 200, 0, 32);
      this->clearVis();
      this->vis1height = noteNumber*4;
      this->vis2rad = speedNumber;
      this->drawVis();         
      display.display(); 
      if (digitalRead(this->button2Pin) == LOW){
        
        if (speedNumber == 32){
         tone(buzzerPin, this->freq);
        }
        else {

          tone(buzzerPin, this->freq, playtime);
          delay(playtime*1.3);
        }
      }
      else {
        noTone(buzzerPin);
      }



        }     
      }
    };


  




class EtchGame {// Etch a sketch Game
    public:
      Ball ball;
      int leftBumperPin;
      int rightBumperPin;   
      int button1Pin; 
      int button1State;
      int button2Pin;
      int button2State;
      int polarity;
      EtchGame(int inleftBumperPin, int inrightBumperPin, int inbutton1Pin, int inbutton2Pin){      
        this->leftBumperPin = inleftBumperPin;
        this->rightBumperPin = inrightBumperPin;
        this->button1Pin = inbutton1Pin;
        this->button2Pin = inbutton2Pin;
        ball.radius = 0.5;    
        button1State = 1;  
      }
      bool playGame(){
        while(1){
        
        int newbutton1State = digitalRead(this->button1Pin);
        int newbutton2State = digitalRead(this->button2Pin);
        if ((newbutton1State == LOW)&&(button1State ==0)) {       //Exit Button
          button1State = 1;
   
          delay(10);
          return(0);
      }
         else if (newbutton1State == HIGH){
          button1State = 0;
      }

      if ((newbutton2State == LOW)&&(button2State ==0)) {       //Erase
        button2State = 1;
        display.clearDisplay();
        display.display();
        delay(10);
      }

        else if (newbutton2State == HIGH) {
          button2State = 0;
        }

         ball.x = map(1024 - analogRead(leftBumperPin), 0, 1023, ball.radius, display.width()-ball.radius);
         ball.y = map(1024 - analogRead(rightBumperPin), 0, 1023, ball.radius, display.height()-ball.radius);
         ball.draw();
         display.display();
        }     
      }
      void resetGame(){
        display.clearDisplay();
      }
};


     Menu maineMenu(button1Pin, button2Pin, rightBumperPin);
     EtchGame newEtchGame(leftBumperPin, rightBumperPin, button2Pin, button1Pin);
     SongPlayer newSongGame(leftBumperPin, rightBumperPin, button2Pin, button1Pin);
     PongGame newPongGame(leftBumperPin, rightBumperPin, button2Pin, 5);

void showUFO(){ 
 display.clearDisplay();
 display.display();
 tone(6, 1000, 100);
 delay(100); 
 tone(6, 500, 130);
 delay(130);
 tone(6, 1000, 100);
 delay(400);
 tone(6, 250, 200);


 
 Sprite ufo;
 ufo.x = 10;
 ufo.y = 10;
 ufo.velx = 1.5;
 ufo.vely = 0.3;
 ufo.accely = 0.01;
 while(millis() < 1000){
  ufo.clr();
  ufo.updatePosition();
  ufo.draw();
  display.display();
 }
 ufo.accely=-0.02;
 ufo.accelx=0.3;
 
  while (millis() < 1500) {
    ufo.clr();
    ufo.updatePosition();
    ufo.draw();
    display.display();
  }
  ufo.accelx = -0.9;
  ufo.accely = 0.1;
  

  while (millis() < 2600) {
    ufo.clr();
    ufo.updatePosition();
    ufo.draw();
    display.display();
  }
  display.clearDisplay();
  return(0);
}

void displaytext() {
  display.setCursor(0, 0);
  display.setTextColor(WHITE); 

  display.println(F("welcome!"));
  display.println(F("i am a computer. "));
  display.println(F("there is an infinite"));
  display.println(F("world of"));
  display.println(F("possibilities!"));
  display.println(F("Please call me VI TO"));
  display.setTextSize(1);
  display.display();
  int button2State = 1;
  while(1){
    int newbutton2State = digitalRead(button2Pin);
    if ((newbutton2State == LOW)&&(button2State ==0)) { 
      button2State = 1;
      delay(10);
      return(0);
      }
    else if (newbutton2State == HIGH){
      button2State = 0;
      }
    }
}
   
void setup()   {   

  randomSeed(analogRead(0));


  display.begin(0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  display.clearDisplay();
  showUFO();            // play animation
  display.display();
  display.clearDisplay();
  display.display();

}



void loop() {

  int result = maineMenu.show();
  switch(result){
   case 0:
    display.clearDisplay();
    
    newPongGame.resetGame();
    newPongGame.playGame();
    break;

   case 1:
    display.clearDisplay();
    newSongGame.playGame();
    break;
 
   case 2:
   display.clearDisplay();
   newEtchGame.resetGame();
   newEtchGame.playGame();
   break;

   case 3:
   display.clearDisplay();
   displaytext();
   break;
   case 99:
    //result = maineMenu.show();
   default:
   break;
  }

}
