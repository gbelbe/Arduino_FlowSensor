void alarm(){
  
  
   int interval = (3 / totalMilliLitres) * 2000;
        
        digitalWrite( beeper, HIGH);
        delay(1000);
        digitalWrite( beeper, LOW);
        delay(interval);
        
        
}
  
  
