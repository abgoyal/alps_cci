#ifndef _KD_CAMERA_HW_H_
#define _KD_CAMERA_HW_H_
 


#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>


//
//Power 
#define CAMERA_POWER_VCAM_A  MT6323_POWER_LDO_VCAMA
#define CAMERA_POWER_VCAM_D  MT6323_POWER_LDO_VCAMD     //VCAMD
#define CAMERA_POWER_VCAM_A2 MT6323_POWER_LDO_VCAM_AF   //VCAMAF
#define CAMERA_POWER_VCAM_D2 MT6323_POWER_LDO_VCAM_IO   //VCAMIO

#ifndef GPIO_MAIN_CAMERA_12V_POWER_CTRL_PIN 
#define GPIO_MAIN_CAMERA_12V_POWER_CTRL_PIN GPIO_CAMERA_LDO_EN_PIN //
#endif 

#endif 
