static uint32_t tmr;
int period = 1000;
bool error_run = false;   //ошибка запуска
int status_run_gen = 0;   //статус работы генератора 
int engine_stop = 0;      //кол-во остановок
const float Vref =5.0;    //опорное напряжение 
float R1 = 100000.0;      //сопротивление R1 (100K)
float R2 = 10000.0;       //сопротивление R2 (10K)
float charge_level = 0.0; //уровень заряда батарей
float vout = 0.0;
float vout_check = 0.0;
const int num_read_vout_check = 5;  // количество усреднений для средних арифм.
int min_charge_lavel = 49;  //минимальный уровень заряда 47
int max_charge_lavel = 53;  //максимальный  

void setup() {
  pinMode(A0,OUTPUT);  // реле зпуска
  pinMode(3,INPUT);    // статус работы генератора
  pinMode(A1,INPUT);   // напряжение на батареи
int engine_start = 0;  //кол-во запусков
  pinMode(A2,OUTPUT);  // индикация ошибка

  Serial.begin(9600);
  Serial.setTimeout(50);
  Serial.println("status_gen,charge_level,error_run,engine_start,engine_stop");
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
}

void loop() {
  if (millis() - tmr >= period) {
    tmr = millis();
    vout =  (analogRead(A1) * Vref) / 1024.0;
    float sum = 0.0;                      
    for (int i = 0; i < num_read_vout_check; i++)  // согласно количеству усреднений
       sum += (analogRead(A1) * Vref) / 1024.0;                
       delay(1000);  
    vout_check = sum /num_read_vout_check;

    if ((vout-vout_check)>-0.5 and (vout-vout_check) < 0.5  ){    
          charge_level = vout / (R2/(R1+R2));
    }      
    else {
      Serial.print( vout / (R2/(R1+R2)));Serial.println(' ');
      Serial.print( vout_check / (R2/(R1+R2)));Serial.println(' ');
      Serial.print(vout_check);Serial.println(' ');
      Serial.print(vout);Serial.println(' ');  
      Serial.print("Неверное значение");Serial.print(' ');
      charge_level=0;// обнуляем
    }           
    if (charge_level<0.01) {
      charge_level=0;// обнуляем нежелательное значение
    }
    if (error_run==true){
       digitalWrite(A2, HIGH);
    }
    status_run_gen = !digitalRead(3);              
    Serial.print(status_run_gen);Serial.print(' ');
    Serial.print(charge_level);Serial.print(' ');
    Serial.print(error_run);Serial.print(' ');
    Serial.print(engine_start);Serial.print(' ');
    Serial.println(engine_stop);

    
    if (status_run_gen==1 and charge_level<max_charge_lavel and engine_start>0) {   //если генератор запущен, уровень заряда ниже max_charge_lavel, счетчик запуска отличен от 0   
         Serial.println("Генератор запущен жду");         
         digitalWrite(A3, HIGH);
        // delay(120000);
         delay(3600000); // ждем 1 часа
         Serial.println("Генератор запущен жду ещё час");
         delay(3600000); // ждем 1 часа
         digitalWrite(A3, LOW);
         return;   
    }    
    if(charge_level < min_charge_lavel ){        
      if (status_run_gen==1 and engine_start>0 ){
        delay(1000);
        return;
      }
      else {
        engine_start = engine_start + 1;
        if(engine_start>1){          
          Serial.println("Генератор не запущен");
          Serial.println(engine_start);
          if (engine_start == 60 or  engine_start == 120 or  engine_start == 210 ){
            error_run = true;
            Serial.print("Повторный запуск");
            btn_gen(); //start gen 
          }
          if (engine_start>210){
              Serial.println("Генератор не запущен, попытки запуска исчерпаны");
              digitalWrite(A2, HIGH);// ошибка запуска        
          }
          return;
        }
        else {
          Serial.println("Start gen");
          btn_gen(); //start gen
          engine_stop = 0;         
          return;  
        } 
      }
    }
    else {
      if (status_run_gen==1 and engine_start>0){
        error_run = false; //сбрасываем ошибку запуска если генератор работает
        engine_start = 0;       
        engine_stop = engine_stop + 1;
        if(engine_stop > 1){        
          Serial.println("Генератор еще не остановлен");
          Serial.println(engine_stop);
          if (engine_stop == 60){
            error_run = true;
            Serial.print("Повторная остановка");
            btn_gen(); //stop gen 
          } 
          if (engine_stop>60){
              digitalWrite(A2, HIGH);//ошибка остановки
          }
          return;
        }
        else{
          Serial.println("Stop gen");         
          btn_gen(); //stop gen
          engine_start = 0;
          return;
        }  
      }
      else {       
        return;     
      }          
    }    
  }  
}     

void btn_gen(){
  digitalWrite(A0, HIGH);
  digitalWrite(A2, HIGH);
  delay(3000); 
  digitalWrite(A0, LOW);
  digitalWrite(A2, LOW);
  delay(10000);
  void loop();               
}
