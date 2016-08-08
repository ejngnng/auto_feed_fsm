# Auto_feed_fsm
###auto feed system for laser cut, the controller of laser cut is MPC6515

This controller is arduino nano 328p 

input is 3 keys:

                key1 --- estop
                key2 --- config
                key3 --- start
                
output is 4 relays:

                relay1 --- power on/off sensor
                relay2 --- power on/off feed motor
                relay3 --- connect to start cut user interface of laser cut control panel 
                relay4 --- connect to stop cut user interface of laser cut control panel
