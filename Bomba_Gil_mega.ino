  
 #include <EEPROM.h>  
 #include <TimeAlarms.h>
 #include <DS3232RTC.h>  
 #include <avr/wdt.h>
 #define minutos  60000;    

 
 //    Leds 
 int ledmangueira_pin = 17;
 int ledmanual1_pin = 3;
 int ledmanual2_pin = 4;
 int ledmanual3_pin = 5;
 int ledauto1_pin = 6;
 int ledauto2_pin = 7;
 int ledauto3_pin = 8;

 //     Inputs
 int manualsw1_pin = 9;
 int manualsw2_pin = 10;
 int manualsw3_pin = 11;
 int autosw1_pin = 12;
 int autosw2_pin = 15; // skip d13
 int autosw3_pin = 14;
 int startsw_pin = 16;
 int stopsw_pin = 18;
 
 //     Outputs
 int valve1_pin = 44;
 int valve2_pin = 45;
 int valve3_pin = 46;
 int valvemangueira_pin = 47;
 int motorrelay_pin = 48;


 //    running variables
 unsigned long timestart0_millis = 0;
 unsigned long timestart1_millis = 0;
 unsigned long timestart2_millis = 0;
 unsigned long timestart3_millis = 0; 
 unsigned long current_millis = 0;
 unsigned long timerautosw = 0;
 unsigned long timeset0_millis = 0;
 unsigned long timeset1_millis = 0;
 unsigned long timeset2_millis = 0;
 unsigned long timeset3_millis = 0;
 unsigned long inrush_millis = 0;
   
 int start0 = 0; 
 int start1 = 0; 
 int start2 = 0; 
 int start3 = 0;
 int run1 = 0;
 int run2 = 0;
 int run3 = 0; 
 int touched = 0;
  
 boolean mangueira_bool = false;
 boolean aspersor_bool = false;
 boolean auto1_bool = false; 
 boolean auto2_bool = false; 
 boolean auto3_bool = false;
 
 DS3232RTC myRTC;
 
void setup() {
  //pins and settings declared 
     myRTC.begin();    
     setSyncProvider(myRTC.get);

     pinMode(ledmangueira_pin, OUTPUT);
     pinMode(ledauto1_pin, OUTPUT);
     pinMode(ledauto2_pin, OUTPUT);
     pinMode(ledauto3_pin, OUTPUT); 
     pinMode(ledmanual1_pin, OUTPUT);  
     pinMode(ledmanual2_pin, OUTPUT);
     pinMode(ledmanual3_pin, OUTPUT);
     
     pinMode(manualsw1_pin, INPUT_PULLUP);
     pinMode(manualsw2_pin, INPUT_PULLUP);
     pinMode(manualsw3_pin, INPUT_PULLUP);
     pinMode(autosw1_pin, INPUT_PULLUP);  
     pinMode(autosw2_pin, INPUT_PULLUP);
     pinMode(autosw3_pin, INPUT_PULLUP);
     pinMode(startsw_pin, INPUT_PULLUP);
     pinMode(stopsw_pin, INPUT_PULLUP); 

     pinMode(valve1_pin, OUTPUT);
     pinMode(valve2_pin, OUTPUT);
     pinMode(valve3_pin, OUTPUT);
     pinMode(valvemangueira_pin, OUTPUT);
     pinMode(motorrelay_pin, OUTPUT);        

     digitalWrite(ledmangueira_pin, LOW);
     digitalWrite(ledauto1_pin, LOW);
     digitalWrite(ledauto2_pin, LOW); 
     digitalWrite(ledauto3_pin, LOW);   
     digitalWrite(motorrelay_pin, LOW);  
     
     
     //Eeprom
     if(EEPROM.read(1) == 1) auto1_bool = true; 
     if(EEPROM.read(2) == 1) auto2_bool = true;
     if(EEPROM.read(3) == 1) auto3_bool = true;         
     
     //Leds
     if(auto1_bool == true) digitalWrite(ledauto1_pin, HIGH);
     if(auto2_bool == true) digitalWrite(ledauto2_pin, HIGH);
     if(auto3_bool == true) digitalWrite(ledauto3_pin, HIGH); 
     
     //alarms and dst
    
      Alarm.alarmRepeat(dowFriday,9,0,0,WeeklyAlarmdst);  // TEMPO EM UTC !!  hora real verao(dst) = utc   || hora real inverno = utc-1
      Alarm.alarmRepeat(dowFriday,10,0,0,WeeklyAlarm); // Alarme de inverno
      
      myRTC.writeRTC(16,-30); // RTC aging offset (+1 = 0.1ppm) Fazer as contas para afinar na proxima "manutenção" Tava atrasado o relogio entao valor negativo para o relogio acelerar
      wdt_enable(WDTO_8S);  // watchdog       
 }
 
 
void loop() { 
  // Read pins
  readpins_func();
  readauto_func();
  runmotor_func();
  
     //        WATCHDOG
     
  wdt_reset(); 

     //        RTC 

  Alarm.delay(10);  // runs scheduler  SHOULD BE LAST THING !     
} 

 
unsigned long milli(unsigned long teste){  // millis since "teste"  // Do i even need this ??
  unsigned long timerlong;
  current_millis = millis();
  timerlong = current_millis - teste;
  return timerlong;
}

void readpins_func(){
  
    //  Read switches states
    
  if(digitalRead(manualsw1_pin) == HIGH && digitalRead(manualsw2_pin) == HIGH && digitalRead(manualsw3_pin) == HIGH && mangueira_bool == false){
    touched = 0;
  }

  if(touched == 0){    
    if(digitalRead(manualsw1_pin) == LOW){
      touched = 1;    
     if(run1 == 0){
      start1 = 1;
      timeset1_millis = 30 * minutos;
      digitalWrite(ledmanual1_pin, HIGH);
      }
     else{
      run1 = 0;
      digitalWrite(ledmanual1_pin, LOW);
      digitalWrite(valve1_pin, LOW);      
    }
   }
   
   if(digitalRead(manualsw2_pin) == LOW){
      touched = 1;         
     if(run2 == 0){
      start2 = 1;
      timeset2_millis = 30 * minutos;
      digitalWrite(ledmanual2_pin, HIGH);
      }
     else{
      run2 = 0;
      digitalWrite(ledmanual2_pin, LOW);
      digitalWrite(valve2_pin, LOW);      
    }
   }
   
   if(digitalRead(manualsw3_pin) == LOW){
      touched = 1;   
     if(run3 == 0){
      start3 = 1;
      timeset3_millis = 30 * minutos;
      digitalWrite(ledmanual3_pin, HIGH);
      }
     else{
      run3 = 0;
      digitalWrite(ledmanual3_pin, LOW);
      digitalWrite(valve3_pin, LOW);      
    }
   }    
  }   

          // Start and Stop switches
 
   if(digitalRead(startsw_pin) == LOW && aspersor_bool == false && mangueira_bool == false){ // "bool == false" Makes it so it only works when everything is off                
     start0 = 1;
     touched = 1;     
    }
    
   if(digitalRead(stopsw_pin) == LOW) { 
    mangueira_bool = false;
    aspersor_bool = false;    
    run1 = 0;
    run2 = 0;
    run3 = 0; 
    start0 = 0;
    start1 = 0;
    start2 = 0;
    start3 = 0;
    digitalWrite(ledmangueira_pin, LOW);         
    digitalWrite(ledmanual1_pin, LOW);
    digitalWrite(ledmanual2_pin, LOW);
    digitalWrite(ledmanual3_pin, LOW);
    digitalWrite(valve1_pin, LOW); 
    digitalWrite(valve2_pin, LOW); 
    digitalWrite(valve3_pin, LOW); 
    digitalWrite(valvemangueira_pin, LOW);  
    digitalWrite(motorrelay_pin, LOW); 
              
   }    
}

void readauto_func(){
  if(digitalRead(autosw1_pin) == HIGH && digitalRead(autosw2_pin) == HIGH && digitalRead(autosw3_pin) == HIGH ) timerautosw = millis();     //reset timer   
 
  if(digitalRead(autosw1_pin) == LOW || digitalRead(autosw2_pin) == LOW || digitalRead(autosw3_pin) == LOW){
    if(milli(timerautosw) > 2500){
      
      if(digitalRead(autosw1_pin) == LOW){
        timerautosw = millis();
        if(auto1_bool == true){
          auto1_bool = false;
          EEPROM.write(1,0);
          digitalWrite(ledauto1_pin, LOW); 
        }
        else {
          auto1_bool = true;
          EEPROM.write(1,1);
          digitalWrite(ledauto1_pin, HIGH);
        }
      }
      
      if(digitalRead(autosw2_pin) == LOW){
        timerautosw = millis();
        if(auto2_bool == true){
          auto2_bool = false;
          EEPROM.write(2,0);
          digitalWrite(ledauto2_pin, LOW); 
        }
        else {
          auto2_bool = true;
          EEPROM.write(2,1);
          digitalWrite(ledauto2_pin, HIGH);
        }
      } 
           
      if(digitalRead(autosw3_pin) == LOW){
        timerautosw = millis();
        if(auto3_bool == true){
          auto3_bool = false;
          EEPROM.write(3,0);
          digitalWrite(ledauto3_pin, LOW); 
        }
        else {
          auto3_bool = true;
          EEPROM.write(3,1);
          digitalWrite(ledauto3_pin, HIGH);
        }
      }      
    }
  }
}

void runmotor_func(){

      //Start mangueira, only if aspersor is off
  
  if(start0 == 1 && aspersor_bool == false){
    start0 = 0; 
    mangueira_bool = true;  
    timestart0_millis = millis();    
    timeset0_millis = 40 * minutos;
    digitalWrite(valvemangueira_pin, HIGH);
    digitalWrite(ledmangueira_pin, HIGH);    
  }

      // Check mangueira timer

  if( milli(timestart0_millis) > timeset0_millis && mangueira_bool == true){
    digitalWrite(ledmangueira_pin, LOW);
    digitalWrite(valvemangueira_pin, LOW);
    mangueira_bool = false;
  }

      //Start aspersor, start timers
  
  if(mangueira_bool == false){ 
    if(start1 == 1 && milli(inrush_millis) > 2000){
      start1 = 0;
      run1 = 1;
      inrush_millis = millis();
      digitalWrite(valve1_pin, HIGH);
      aspersor_bool = true;
      timestart1_millis = millis();               
    }
    if(start2 == 1 && milli(inrush_millis) > 2000){
      start2 = 0;
      run2 = 1;
      inrush_millis = millis();
      digitalWrite(valve2_pin, HIGH);
      aspersor_bool = true;   
      timestart2_millis = millis();   
    }
    if(start3 == 1 && milli(inrush_millis) > 2000){
      start3 = 0;
      run3 = 1;
      inrush_millis = millis();
      digitalWrite(valve3_pin, HIGH);
      aspersor_bool = true;
      timestart3_millis = millis();      
    }
  }
  
      //Check timers
      
  if( milli(timestart1_millis) > timeset1_millis && run1 == 1){  
    run1 = 0;
    digitalWrite(valve1_pin, LOW);
    digitalWrite(ledmanual1_pin, LOW);
  }
  if( milli(timestart2_millis) > timeset2_millis && run2 == 1){
    run2 = 0;
    digitalWrite(valve2_pin, LOW);
    digitalWrite(ledmanual2_pin, LOW);
  }
  if( milli(timestart3_millis) > timeset3_millis && run3 == 1){
    run3 = 0;
    digitalWrite(valve3_pin, LOW);
    digitalWrite(ledmanual3_pin, LOW);
  }
  
      // Turn off motor if all queue is ended
      
  if(run1 == 0 && run2 == 0 && run3 == 0 && aspersor_bool == true){
    aspersor_bool = false;
    digitalWrite(valve1_pin, LOW);
    digitalWrite(valve2_pin, LOW);
    digitalWrite(valve3_pin, LOW);
    digitalWrite(motorrelay_pin, LOW);
  }
  
      // Turn on or off motor 
      
  if(mangueira_bool == true || aspersor_bool == true ){
    digitalWrite(motorrelay_pin, HIGH);    
  }
  if(mangueira_bool == false && aspersor_bool == false){  
    digitalWrite(motorrelay_pin, LOW);
  }
}
 
void WeeklyAlarm(){  
   if(month() < 4 || month() > 10){
    alarm_func();
    }
 } 

void WeeklyAlarmdst(){  // alarme verao
  if(month() > 3 && month() < 11){
    alarm_func();
    }
   
 }

void alarm_func(){
  unsigned long timesetalarm=0;
  
  timesetalarm = 20*minutos ;      //Nov,Dez,Jan,Fev,Mar
  if(month() > 3 && month() < 11){
    timesetalarm = 30*minutos;      //Abr,Mai,Set,Out
    }
  if(month() > 5 && month() < 9){
    timesetalarm = 40*minutos;  //Jun,Jul,Ago
    }
  
  if(auto1_bool == 1){
    start1 = 1;
    timeset1_millis = timesetalarm;    
    digitalWrite(ledmanual1_pin, HIGH);
  }
  if(auto2_bool == 1){
    start2 = 1;
    timeset2_millis = timesetalarm;    
    digitalWrite(ledmanual2_pin, HIGH);
  }
  if(auto3_bool == 1){
    start3 = 1;
    timeset3_millis = timesetalarm;    
    digitalWrite(ledmanual3_pin, HIGH);
  }
              
}   

 
