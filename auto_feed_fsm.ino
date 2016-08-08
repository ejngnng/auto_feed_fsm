/*
 * Description: auto feed system
 * Hardware   : Arduino nano 328p
 * Version    : V1.0
 * Author     : Ninja
 * Date       : 2016-06
 */
#include<stdio.h>
#include<stdlib.h>
//#include <U8glib.h>

#define SENSOR     5   //位移传感器信号
#define SENSOR_POWER 7 //位移传感器电源开关
#define LASER      10  //加工停止信号

#define CUT_START  9   //加工启动面板
#define CUT_STOP   8   //加工停止面板

#define KEY_START  19   //开始按钮
#define KEY_CONFIG 18   //参数配置按钮
#define KEY_ESTOP  17   //急停按钮

#define MOTOR      6   //送料电机

#define DEBUG      1   //串口DEBUG开关

#if 0
#define CS         5   //LCD12864 CS
#define RESET      7   //LCD12864 RESET

U8GLIB_ST7920_128X64_1X LCD(CS,RESET);   // HW SPI: SCK MOSI

#endif

/*
 * 函数返回值
 */
typedef enum rs_s{
  RS_OK,
  RS_FAIL
}rs_t;

/*
 *信号状态
 */
typedef enum signal_state_s{
  ON,
  OFF
}signal_state_t;

/*
 * 系统状态
 */
typedef enum state_s{
  INIT,
  CONFIG,
  READY_CUT,
  CUTTING,
  FEEDING,
  ESTOP
}state_t;

typedef struct feed_obj_s{
  unsigned int cut_times;
  unsigned char state;
}feed_obj_t;


typedef struct action_s{
  rs_t (*sys_init)(feed_obj_t *feed_obj);
  rs_t (*sys_config)(feed_obj_t *feed_obj);
  rs_t (*sys_ready)(feed_obj_t *feed_obj);
  rs_t (*sys_cutting)(feed_obj_t *feed_obj);
  rs_t (*sys_feeding)(feed_obj_t *feed_obj);
  rs_t (*sys_estop)(feed_obj_t *feed_obj);
  void (*sys_key)(feed_obj_t *feed_obj);
}action_t;


/*
 * 启动送料电机
 */
void motor_start(){
  if(DEBUG){
    Serial.println("motor_start\n");
    delay(100);
  }
  digitalWrite(MOTOR,HIGH);
  delay(2);
}

/*
 * 停止送料电机
 */
void motor_stop(){
   if(DEBUG){
    Serial.println("motor_stop\n");
    delay(100);
  }
  digitalWrite(MOTOR,LOW);
  delay(2);
}

/*
 * 清除加工参数
 */
void reset_cut_params(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("reset_cut_params");
    delay(100);
  }
  feed_obj->cut_times = 0;
  feed_obj->state = INIT;
  sensor_power_off();
}
 
/*
 * 位移检测传感器
 */
signal_state_t sensor_read(){
  if(DEBUG){
      Serial.println("sensor_read\n");
      delay(100);
  }
  if(digitalRead(SENSOR) != HIGH){
    return OFF;
  }
  else{
    return ON;
  } 
}

/*
 * 加工停止信号检测
 */
signal_state_t laser_stop(){
  if(DEBUG){
    Serial.println("laser_stop\n");
    delay(100);
  }
  if(digitalRead(LASER) != HIGH){
    return OFF;
  }
  else{
    return ON;
  }
}

/*
 * 自定义面板启动加工
 */
void cut_start(){
  if(DEBUG){
    Serial.println("cut_start\n");
    delay(100);
  }
  digitalWrite(CUT_START,HIGH);
  delay(2);
}

/*
 * 自定义面板停止加工
 */
void cut_stop(){
  if(DEBUG){
    Serial.println("cut_stop\n");
    delay(100);
  }
  digitalWrite(CUT_STOP,HIGH);
  delay(2);
}

/*
 * 开 位移传感器
 */
void sensor_power_on(){
  if(DEBUG){
    Serial.println("sensor_power_on\n");
    delay(100);
  }
  digitalWrite(SENSOR_POWER,LOW);
  delay(2);
}

/*
 * 关 位移传感器
 */
void sensor_power_off(){
  if(DEBUG){
    Serial.println("sensor_power_off\n");
    delay(100);
  }
  digitalWrite(SENSOR_POWER,HIGH);
  delay(2);
}

/*
 * 初始化系统
 */
rs_t action_init(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("action_init");
    delay(100);
  }
  if(NULL == feed_obj){
    return RS_FAIL;
  }
  
  if(feed_obj->state != INIT){
    return RS_FAIL;
  }
  motor_stop();
  reset_cut_params(feed_obj);
  //feed_obj->state = CONFIG;
  while(!Serial){
    
  }
  Serial.println("\t");
  Serial.println("***********自动送料系统************\t");
  Serial.println("Version:V1.0 \t");
  Serial.println("Author :Ninja \t");
  Serial.println("Date   :2016-06\t");
  Serial.println("*********************************\t"); 
  return RS_OK;
}

/*
 * 加工参数配置
 */
rs_t action_config(feed_obj_t *feed_obj){
  String val = "";
  unsigned int i = 0;
  if(DEBUG){
    Serial.println("action_config\n");
    delay(100);
  }
  if(NULL == feed_obj){
    return RS_FAIL;
  }
  
  if(feed_obj->state != CONFIG){
    return RS_FAIL;
  }

  Serial.println("请输入加工次数:\t");
  while(Serial.available() == 0){
    //等待串口输入
  }
  while(Serial.available() > 0){
    val += char(Serial.read());
    delay(2);
  }
  if(val.length() > 5 || val.toInt() > 65535){
    Serial.println("加工次数超出范围");
    feed_obj->state = CONFIG;
    return RS_FAIL;
  }
  for(i=0;i<val.length();i++){
    if(val[i] < '0' || val[i] > '9'){
      Serial.println("加工参数只能输入数字");
      feed_obj->state = CONFIG;
      return RS_FAIL;
    }
  }
  if(val.toInt() == 0){
    Serial.println("加工参数不能为0");
    feed_obj->state = CONFIG;
    return RS_FAIL;
  }
  feed_obj->cut_times = val.toInt();
  feed_obj->state = READY_CUT;
  Serial.println(feed_obj->cut_times,DEC);
  Serial.println("请按加工按钮\t");
  if(DEBUG){
    Serial.println(feed_obj->cut_times,DEC);
    delay(1000);
  }
  return RS_OK;
}

/*
 * 准备加工
 */
rs_t action_ready(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("action_ready\n");
    delay(100);
  }
  feed_obj->state = READY_CUT;
  return RS_OK;
}

/*
 * 启动加工
 */
rs_t action_cutting(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("action_cutting\n");
    delay(100);
  }
  if(NULL == feed_obj){
    return RS_FAIL;
  }
  if(feed_obj->cut_times == 0){
    feed_obj->state = CONFIG;
    return RS_FAIL;
  }
  cut_start();
  while(laser_stop() != OFF){
    //等待加工结束
  }
  feed_obj->state = FEEDING;
  (feed_obj->cut_times)--;
  if(DEBUG){
    Serial.println(feed_obj->cut_times,DEC);
    delay(1000);
  }
  return RS_OK;
}

/*
 * 启动送料
 */
rs_t action_feeding(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("action_feeding\n");
    delay(100);
  }
  if(feed_obj->state != FEEDING){
    return RS_FAIL;
  }
  if(feed_obj->cut_times == 0){
    feed_obj->state = CONFIG;
    return RS_OK;
  }
  motor_start();
  while(sensor_read() != OFF){
    //等待位移检测信号
  }
  sensor_power_on();
  motor_stop();
  feed_obj->state = CUTTING;
  sensor_power_off();
  return RS_OK;
}

/*
 * 急停
 */
rs_t action_estop(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("action_estop\n");
    delay(100);
  }
  if(NULL == feed_obj){
    return RS_FAIL;
  }
  cut_stop();
  feed_obj->cut_times = 0;
  feed_obj->state = INIT;
  return RS_OK;
}
 
void func_init(action_t *action){
  if(DEBUG){
    Serial.println("func_init\n");
    delay(100);
  }
  action->sys_init = action_init;
  action->sys_config = action_config;
  action->sys_ready = action_ready;
  action->sys_cutting = action_cutting;
  action->sys_feeding = action_feeding;
  action->sys_estop = action_estop;
  action->sys_key = key_scan;
}

/*
 * 控制引脚初始化
 */
void port_init(){
  pinMode(SENSOR,INPUT);
  digitalWrite(SENSOR,HIGH);
  pinMode(LASER,INPUT);
  digitalWrite(LASER,HIGH);

  pinMode(SENSOR_POWER,OUTPUT);
  digitalWrite(SENSOR_POWER,HIGH);

  pinMode(CUT_START,OUTPUT);
  digitalWrite(CUT_START,HIGH);
  pinMode(CUT_STOP,OUTPUT);
  digitalWrite(CUT_STOP,HIGH);

  pinMode(KEY_START,INPUT);
  digitalWrite(KEY_START,HIGH);
  pinMode(KEY_CONFIG,INPUT);
  digitalWrite(KEY_CONFIG,HIGH);
  pinMode(KEY_ESTOP,INPUT);
  digitalWrite(KEY_ESTOP,HIGH);

  pinMode(MOTOR,OUTPUT);
  digitalWrite(MOTOR,HIGH);
}

/*
 * 操作按钮检测
 */
void key_scan(feed_obj_t *feed_obj){
  if(DEBUG){
    Serial.println("key_scan\n");
  }
  if(digitalRead(KEY_START) == LOW){
    delay(10);
    if(digitalRead(KEY_START) == LOW){
      feed_obj->state = CUTTING;
      if(DEBUG){
        Serial.println("KEY_START");
      }
    }
  }
  else if(digitalRead(KEY_CONFIG) == LOW){
    delay(10);
    if(digitalRead(KEY_CONFIG) == LOW){
      feed_obj->state = CONFIG;
      if(DEBUG){
        Serial.println("KEY_CONFIG");
      }
    }
  }
  else if(digitalRead(KEY_ESTOP) == LOW){
    delay(10);
    if(digitalRead(KEY_ESTOP) == LOW){
      feed_obj->state = ESTOP;
      Serial.println("KEY_ESTOP");
    }
  }
  else{
    //feed_obj->state = INIT;
    return;
  }
}

void setup() {
  // put your setup code here, to run once:
  port_init();
  Serial.begin(9600);
 // while(!Serial){
    //等待串口连接
 // }

}

void loop() {
  // put your main code here, to run repeatedly:
  feed_obj_t feed_obj;
  action_t   action;
  state_t    state;
  
  memset(&feed_obj,0,sizeof(feed_obj_t));
  memset(&action,0,sizeof(action_t));
  
  feed_obj.state = INIT;
  func_init(&action);
  
  while(1){
    action.sys_key(&feed_obj);
    switch(feed_obj.state){
      case INIT:
            action.sys_init(&feed_obj);
            break;
      case CONFIG:
            action.sys_config(&feed_obj);
            break;
      case READY_CUT:
            action.sys_ready(&feed_obj);
            break;
      case CUTTING:
            action.sys_cutting(&feed_obj);
            break;
      case FEEDING:
            action.sys_feeding(&feed_obj);
            break;
      case ESTOP:
            action.sys_estop(&feed_obj);
            break;
      default:
            break;
    }
  }

}
