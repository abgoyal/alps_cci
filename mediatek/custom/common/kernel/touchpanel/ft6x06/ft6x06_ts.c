/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft6x06 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include "ft6x06_ts.h"
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>

#include "tpd_custom_ft5206.h"
#define FTS_CTL_IIC
//#define SYSFS_DEBUG
#define FTS_APK_DEBUG
#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif
#ifdef SYSFS_DEBUG
#include "ft6x06_ex_fun.h"
#endif
#include "tpd.h"
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>


#include <cust_eint.h>
#ifdef MT6575
#include <mach/mt6575_pm_ldo.h>
#include <mach/mt6575_typedefs.h>
#include <mach/mt6575_boot.h>
#endif


#ifdef MT6577
#include <mach/mt6577_pm_ldo.h>
#include <mach/mt6577_typedefs.h>
#include <mach/mt6577_boot.h>
#endif

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>


#include "cust_gpio_usage.h"

#include <linux/proc_fs.h>

struct i2c_client *i2c_client = NULL; //add by zhaofei
struct task_struct *thread = NULL;


struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft6x06_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct ft6x06_platform_data *pdata;
#ifdef CONFIG_PM
	struct early_suspend *early_suspend;
#endif
};

#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02


/*
*ft6x06_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft6x06_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;
	unsigned char addr_temp = 0;

	printk("zhaofei read writelen is %d, readbuf is %x, 0x%x,readlen is %d\n",writelen,readbuf[0],readbuf[1],readlen);
//	if( writelen > 0)
//	{
//		addr_temp = writebuf[0];	
//		ret = i2c_smbus_read_i2c_block_data(i2c_client, addr_temp , readlen, &(readbuf[0]));
//	}
//	else
//	{
//		addr_temp = readbuf[0];	
//		ret = i2c_smbus_read_i2c_block_data(i2c_client, addr_temp , readlen, &(readbuf[0]));
//	}
	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = i2c_client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = i2c_client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(i2c_client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&i2c_client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = i2c_client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(i2c_client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&i2c_client->dev, "%s:i2c read error.\n", __func__);

	}
	return ret;
}
/*write data by i2c*/
int ft6x06_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	printk("zhaofei write : len is %d, buf[0] is %x\n",writelen,writebuf[0]);
	struct i2c_msg msg[] = {
		{
		 .addr = i2c_client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(i2c_client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&i2c_client->dev, "%s i2c write error.\n", __func__);

	//i2c_smbus_write_i2c_block_data(i2c_client, writebuf[0], writelen-1, &writebuf[1]);
	return ret;
}

/*release the point*/
static void ft6x06_ts_release(struct ft6x06_ts_data *data)
{
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_sync(data->input_dev);
}

/*Read touch point information when the interrupt  is asserted.*/
static int ft6x06_read_Touchdata(struct ft6x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft6x06_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	//event->touch_point = buf[2] & 0x0F;

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
	//for (i = 0; i < event->touch_point; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}
	
	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void ft6x06_report_value(struct ft6x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;
	int up_point = 0;
	//int touch_point = 0;

	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */
		if (event->au16_x[i] < data->x_max
		    && event->au16_y[i] < data->y_max) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_PRESSURE,
					 event->pressure);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
					 event->au8_finger_id[i]);
			if (event->au8_touch_event[i] == FTS_POINT_DOWN
			    || event->au8_touch_event[i] == FTS_POINT_CONTACT)\
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
			else {
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
				up_point++;
			}
			//touch_point ++;
		}

		input_mt_sync(data->input_dev);
	}
	if (event->touch_point > 0)
		input_sync(data->input_dev);

	if (event->touch_point == 0)
		ft6x06_ts_release(data);

}

/*The ft6x06 device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft6x06_ts_interrupt(int irq, void *dev_id)
{
	struct ft6x06_ts_data *ft6x06_ts = dev_id;
	int ret = 0;
	disable_irq_nosync(ft6x06_ts->irq);

	ret = ft6x06_read_Touchdata(ft6x06_ts);
	if (ret == 0)
		ft6x06_report_value(ft6x06_ts);

	enable_irq(ft6x06_ts->irq);

	return IRQ_HANDLED;
}

#define TPD_OK 0


#ifdef MT6575 
 extern void mt65xx_eint_unmask(unsigned int line);
 extern void mt65xx_eint_mask(unsigned int line);
 extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
 extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
 extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									  kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									  kal_bool auto_umask);
#endif
#ifdef MT6577
	extern void mt65xx_eint_unmask(unsigned int line);
	extern void mt65xx_eint_mask(unsigned int line);
	extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
	extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

static struct CTPINFO_LIST
{
	char ctpvid;
	char ctpstring[20];
}CTPINFO_LIST;

static struct CTPINFO_LIST ctpinfo[]={
	{0x5D,"Baoming"},
	{0x55,"Laibao"},
	{0x51,"Ofilm"},
	{0x5A,"Truly"},
	{0x80,"each opto"},
	{0x53,"Mutto opto"},
	{0x9E,"wangbo Tech"},
	{0xBA,"JianShuo"},
	{0x79,"IC Original"},
	{0x3F,"HuanDa"},
	{0xB0,"TianYi"}
};


static int tpd_flag = 0;
static int point_num = 0;
static int p_point_num = 0;

struct touch_info {
    int y[5];
    int x[5];
    int p[5];
    int id[5];
    int count;
};
 
#define CTP_PROC_FILE "ctp_version"
static struct proc_dir_entry *g_ctp_proc = NULL;
static char ctp_vid=0;	//vendor ID
static char ctp_fvm=0;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static void tpd_eint_interrupt_handler(void);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
extern struct tpd_device *tpd;
static  void tpd_down(int x, int y, int p) {
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	 printk("D[%4d %4d %4d]\n ", x, y, p);
	 /* track id Start 0 */
       input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 
	 input_mt_sync(tpd->dev);
//     if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
//     {   
//       tpd_button(x, y, 1);  
//     }
	 if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue
	 {
         msleep(50);
		 printk("D virtual key \n");
	 }
	 TPD_EM_PRINT(x, y, x, y, p-1, 1);
 }
 
static  void tpd_up(int x, int y,int *count) {
	 //if(*count>0) {
		 //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
		 input_report_key(tpd->dev, BTN_TOUCH, 0);
		 //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
		 //printk("U[%4d %4d %4d] ", x, y, 0);
		 input_mt_sync(tpd->dev);
		 TPD_EM_PRINT(x, y, x, y, 0, 0);
	//	 (*count)--;
//     if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
//     {   
//        tpd_button(x, y, 0); 
//     }   		 

 }


static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
 {

	int i = 0;
	
	//char data[30] = {0};
	char data[41] = {0};

    u16 high_byte,low_byte;
	u8 report_rate =0;

	p_point_num = point_num;

	i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(data[0]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(data[8]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(data[16]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(data[24]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x20, 8, &(data[32])); //add by zhaofei 2012-05-25 10:40:54
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &(data[40]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x88, 1, &report_rate);
	//TPD_DEBUG("FW version=%x]\n",data[24]);
	
	//TPD_DEBUG("received raw data from touch panel as following:\n");
	//TPD_DEBUG("[data[0]=%x,data[1]= %x ,data[2]=%x ,data[3]=%x ,data[4]=%x ,data[5]=%x]\n",data[0],data[1],data[2],data[3],data[4],data[5]);
	//TPD_DEBUG("[data[9]=%x,data[10]= %x ,data[11]=%x ,data[12]=%x]\n",data[9],data[10],data[11],data[12]);
	//TPD_DEBUG("[data[15]=%x,data[16]= %x ,data[17]=%x ,data[18]=%x]\n",data[15],data[16],data[17],data[18]);


    //    
	 //we have  to re update report rate
    // TPD_DMESG("report rate =%x\n",report_rate);
	 if(report_rate < 8)
	 {
	   report_rate = 0x8;
	   if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	   {
		   TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
	   }
	 }
	 
	
	/* Device Mode[2:0] == 0 :Normal operating Mode*/
	if((data[0] & 0x70) != 0) return false; 

	/*get the number of the touch points*/
	point_num= data[2] & 0x0f;
	
	//TPD_DEBUG("point_num =%d\n",point_num);
	
//	if(point_num == 0) return false;

	   //TPD_DEBUG("Procss raw data...\n");

		
		for(i = 0; i < point_num; i++)
		{
			cinfo->p[i] = data[3+6*i] >> 6; //event flag 
                   cinfo->id[i] = data[3+6*i+2]>>4; //touch id
	       /*get the X coordinate, 2 bytes*/
			high_byte = data[3+6*i];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i + 1];
			cinfo->x[i] = high_byte |low_byte;

				//cinfo->x[i] =  cinfo->x[i] * 480 >> 11; //calibra
		
			/*get the Y coordinate, 2 bytes*/
			
			high_byte = data[3+6*i+2];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i+3];
			cinfo->y[i] = high_byte |low_byte;

			  //cinfo->y[i]=  cinfo->y[i] * 800 >> 11;
		
			cinfo->count++;
			
		}
		//TPD_DEBUG(" cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);	
		//TPD_DEBUG(" cinfo->x[1] = %d, cinfo->y[1] = %d, cinfo->p[1] = %d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1]);		
		//TPD_DEBUG(" cinfo->x[2]= %d, cinfo->y[2]= %d, cinfo->p[2] = %d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);	
		  
	 return true;

 }

static int touch_event_handler(void *unused)
 {
  
    struct touch_info cinfo, pinfo;
	 int i=0;

	 struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	 sched_setscheduler(current, SCHED_RR, &param);
 
	 do
	 {
	  mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
		 set_current_state(TASK_INTERRUPTIBLE); 
		  wait_event_interruptible(waiter,tpd_flag!=0);
						 
			 tpd_flag = 0;
			 
		 set_current_state(TASK_RUNNING);
		 

		  if (tpd_touchinfo(&cinfo, &pinfo)) 
		  {
		    //TPD_DEBUG("point_num = %d\n",point_num);
			TPD_DEBUG_SET_TIME;
			if(point_num >0) 
			{
			    for(i =0; i<point_num; i++)//only support 3 point
			    {

			         tpd_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);
			       
			    }
			    input_sync(tpd->dev);
			}

			else  
            {
			    tpd_up(cinfo.x[0], cinfo.y[0], 0);
                //TPD_DEBUG("release --->\n"); 
                //input_mt_sync(tpd->dev);
                input_sync(tpd->dev);
            }
        }

        if(tpd_mode==12)
        {
           //power down for desence debug
           //power off, need confirm with SA
           hwPowerDown(MT6323_POWER_LDO_VGP1,  "TP");
           hwPowerDown(MT6323_POWER_LDO_VCAM_AF,  "TP");
	    msleep(20);
          
        }

 }while(!kthread_should_stop());
 
	 return 0;
 }

 static void tpd_eint_interrupt_handler(void)
 {
	 //TPD_DEBUG("TPD interrupt has been triggered\n");
	 TPD_DEBUG_PRINT_INT;
	 tpd_flag = 1;
	 wake_up_interruptible(&waiter);
	 
 }

//extern int ft5x0x_autoupgrade(struct i2c_client *client,char* vid,char* firmware);
static int ctp_proc_write(struct file *filp, 
	const char __user *buff, unsigned long len, void *data)
{

	int err=-1;
	//err=ft5x0x_autoupgrade(i2c_client,&ctp_vid,&ctp_fvm);
	if(err!=0)
	printk("[shaohui]Auto upgrade CTPi OK\n");
	else
	printk("[shaohui]Auto upgrade CTP failed!\n");

}


static int ctp_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	int cnt= 0;
	int i=0;
	printk("Enter ctp_proc_read.\n");
	if(off != 0)
		return 0;
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa8, 1, &ctp_vid);
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &ctp_fvm);
	//check vendor


	for(i=0;i<(sizeof(ctpinfo)/sizeof(CTPINFO_LIST));i++)
	{
		if(ctp_vid==ctpinfo[i].ctpvid)
			break;
	}
	
	
	cnt = sprintf(page, "vid:%s,firmware:0x%x\n",ctpinfo[i].ctpstring,ctp_fvm);
	*eof = 1;
	printk("Leave ctp_proc_read. cnt = %d\n", cnt);
	return cnt;
}


extern int ft5x0x_create_apk_debug_channel(struct i2c_client * client);

static int ft6x06_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct ft6x06_platform_data *pdata =
	    (struct ft6x06_platform_data *)client->dev.platform_data;
	struct ft6x06_ts_data *ft6x06_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;
//add by zhaofei 
	int retval = TPD_OK;
	char data;
	u8 report_rate=0;
	int reset_count = 0;

reset_proc:   
	i2c_client = client;

  
    //power on, need confirm with SA
    hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "TP");
	hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1800, "TP");


	#ifdef TPD_CLOSE_POWER_IN_SLEEP	 
	hwPowerDown(TPD_POWER_SOURCE,"TP");
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
	msleep(100);
	#else
	#ifdef MT6573
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
  mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	msleep(100);
	#endif
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(1);
	TPD_DMESG(" ft5306 reset\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	#endif

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
 
	  mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	  mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	  mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1); 
	  mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
 
	msleep(100);
 
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	{
		TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);
#ifdef TPD_RESET_ISSUE_WORKAROUND
        if ( reset_count < TPD_MAX_RESET_COUNT )
        {
            reset_count++;
            goto reset_proc;
        }
#endif
		   return -1; 
	}

	//set report rate 80Hz
	report_rate = 0x8; 
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	{
	    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	    {
		   TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
	    }
		   
	}

//shaohui read CTP version

	i2c_smbus_read_i2c_block_data(i2c_client, 0xa8, 1, &ctp_vid);
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &ctp_fvm);


	printk("[shaohui]Get CTP VID:0x%x,firmware version:0x%x\n",ctp_vid,ctp_fvm);

	tpd_load_status = 1;

#ifdef VELOCITY_CUSTOM_FT5206
	if((err = misc_register(&tpd_misc_device)))
	{
		printk("mtk_tpd: tpd_misc_device register failed\n");
		
	}
	#endif
#ifdef FTS_APK_DEBUG //add by zhaofei
	ft5x0x_create_apk_debug_channel(client);
#endif

	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	 if (IS_ERR(thread))
		 { 
		  retval = PTR_ERR(thread);
		  TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
		}

	TPD_DMESG("ft5206 Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
	
	
	g_ctp_proc = create_proc_entry(CTP_PROC_FILE, 0444, NULL);
	if (g_ctp_proc == NULL) {
		printk("create_proc_entry failed\n");
	} else {
		g_ctp_proc->read_proc = ctp_proc_read;
		g_ctp_proc->write_proc = ctp_proc_write;
		//g_ctp_proc->owner = THIS_MODULE;
		printk("create_proc_entry success\n");
	}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
//		err = -ENODEV;
//		goto exit_check_functionality_failed;
//	}
//
//	ft6x06_ts = kzalloc(sizeof(struct ft6x06_ts_data), GFP_KERNEL);
//
//	if (!ft6x06_ts) {
//		err = -ENOMEM;
//		goto exit_alloc_data_failed;
//	}
//	
//	i2c_set_clientdata(client, ft6x06_ts);
//	ft6x06_ts->irq = client->irq;
//	ft6x06_ts->client = client;
//	ft6x06_ts->pdata = pdata;
//	ft6x06_ts->x_max = pdata->x_max - 1;
//	ft6x06_ts->y_max = pdata->y_max - 1;
//#ifdef CONFIG_PM
//	err = gpio_request(pdata->reset, "ft6x06 reset");
//	if (err < 0) {
//		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
//			__func__);
//		goto exit_request_reset;
//	}
//#endif
//
//	err = request_threaded_irq(client->irq, NULL, ft6x06_ts_interrupt,
//				   pdata->irqflags, client->dev.driver->name,
//				   ft6x06_ts);
//	if (err < 0) {
//		dev_err(&client->dev, "ft6x06_probe: request irq failed\n");
//		goto exit_irq_request_failed;
//	}
//	disable_irq(client->irq);
//
//	input_dev = input_allocate_device();
//	if (!input_dev) {
//		err = -ENOMEM;
//		dev_err(&client->dev, "failed to allocate input device\n");
//		goto exit_input_dev_alloc_failed;
//	}
//
//	ft6x06_ts->input_dev = input_dev;
//
//	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
//	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
//	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
//	set_bit(ABS_MT_PRESSURE, input_dev->absbit);
//
//	input_set_abs_params(input_dev,
//			     ABS_MT_POSITION_X, 0, ft6x06_ts->x_max, 0, 0);
//	input_set_abs_params(input_dev,
//			     ABS_MT_POSITION_Y, 0, ft6x06_ts->y_max, 0, 0);
//	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
//	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
//	input_set_abs_params(input_dev,
//			     ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);
//
//	set_bit(EV_KEY, input_dev->evbit);
//	set_bit(EV_ABS, input_dev->evbit);
//
//	input_dev->name = FT6X06_NAME;
//	err = input_register_device(input_dev);
//	if (err) {
//		dev_err(&client->dev,
//			"ft6x06_ts_probe: failed to register input device: %s\n",
//			dev_name(&client->dev));
//		goto exit_input_register_device_failed;
//	}
//	/*make sure CTP already finish startup process */
//	msleep(150);
//
//	/*get some register information */
//	uc_reg_addr = FT6x06_REG_FW_VER;
//	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
//	dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);
//
//	uc_reg_addr = FT6x06_REG_POINT_RATE;
//	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
//	dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
//		uc_reg_value * 10);
//
//	uc_reg_addr = FT6x06_REG_THGROUP;
//	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
//	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
//		uc_reg_value * 4);
#ifdef SYSFS_DEBUG
	ft6x06_create_sysfs(client);
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif
	enable_irq(client->irq);
	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
	free_irq(client->irq, ft6x06_ts);
#ifdef CONFIG_PM
exit_request_reset:
	gpio_free(ft6x06_ts->pdata->reset);
#endif

exit_irq_request_failed:
	i2c_set_clientdata(client, NULL);
	kfree(ft6x06_ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

#ifdef CONFIG_PM
static void ft6x06_ts_suspend(struct early_suspend *handler)
{
	struct ft6x06_ts_data *ts = container_of(handler, struct ft6x06_ts_data,
						early_suspend);

	dev_dbg(&ts->client->dev, "[FTS]ft6x06 suspend\n");
	disable_irq(ts->pdata->irq);
}

static void ft6x06_ts_resume(struct early_suspend *handler)
{
	struct ft6x06_ts_data *ts = container_of(handler, struct ft6x06_ts_data,
						early_suspend);

	dev_dbg(&ts->client->dev, "[FTS]ft6x06 resume.\n");
	gpio_set_value(ts->pdata->reset, 0);
	msleep(20);
	gpio_set_value(ts->pdata->reset, 1);
	enable_irq(ts->pdata->irq);
}
#else
#define ft6x06_ts_suspend	NULL
#define ft6x06_ts_resume		NULL
#endif

static int __devexit ft6x06_ts_remove(struct i2c_client *client)
{
	struct ft6x06_ts_data *ft6x06_ts;
	ft6x06_ts = i2c_get_clientdata(client);
	input_unregister_device(ft6x06_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(ft6x06_ts->pdata->reset);
	#endif

	#ifdef SYSFS_DEBUG
	ft6x06_release_sysfs(client);
	#endif
	#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
	#endif
	free_irq(client->irq, ft6x06_ts);
	kfree(ft6x06_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft6x06_ts_id[] = {
	{FT6X06_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft6x06_ts_id);

//static struct i2c_driver ft6x06_ts_driver = {
//	.probe = ,
//	.remove = __devexit_p(),
//	//.id_table = ft6x06_ts_id,
//	.suspend = ft6x06_ts_suspend,
//	.resume = ft6x06_ts_resume,
//	.driver = {
//		   .name = FT6X06_NAME,
//		   .owner = THIS_MODULE,
//		   },
//};

static const struct i2c_device_id ft5206_tpd_id[] = {{"ft5206",0},{}};
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 strcpy(info->type, TPD_DEVICE);	
	  return 0;
 }
 

static struct i2c_driver tpd_i2c_driver = {
  .driver = {
	 .name = "ft5206",//.name = TPD_DEVICE,
//	 .owner = THIS_MODULE,
  },
  .probe = ft6x06_ts_probe,
  .remove = __devexit_p(ft6x06_ts_remove),
  .id_table = ft5206_tpd_id,
  .detect = tpd_detect,
//  .address_data = &addr_data,
 };
 
static int tpd_local_init(void)
{
  TPD_DMESG("Focaltech FT5206 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);
 
   if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
  		TPD_DMESG("ft5206 unable to add i2c driver.\n");
      	return -1;
    }
    if(tpd_load_status == 0) 
    {
    	TPD_DMESG("ft5206 add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }
	
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0; 
 }

 static void tpd_resume( struct early_suspend *h )
 {
  //int retval = TPD_OK;
  char data;
 
   TPD_DMESG("TPD wake up\n");
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP"); 
#else
#ifdef MT6573
	mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
	msleep(100);
#endif	
	

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(1);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif
   mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
	
       msleep(20);
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	{
		TPD_DMESG("resume I2C transfer error, line: %d\n", __LINE__);

	}
	tpd_up(0,0,0);
	input_sync(tpd->dev);
	TPD_DMESG("TPD wake up done\n");
	 //return retval;
 }

 static void tpd_suspend( struct early_suspend *h )
 {
	// int retval = TPD_OK;
	 static char data = 0x3;
 
	 TPD_DMESG("TPD enter sleep\n");
	 mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode
#ifdef MT6573
mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);
#endif

#endif
        TPD_DMESG("TPD enter sleep done\n");
	 //return retval;
 } 




static struct i2c_board_info __initdata ft5206_i2c_tpd={ I2C_BOARD_INFO("ft5206", (0x70>>1))};
 static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "FT5206",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
 };

static int __init ft6x06_ts_init(void)
{
	int ret;
//	ret = i2c_add_driver(&ft6x06_ts_driver);
//	if (ret) {
//		printk(KERN_WARNING "Adding ft6x06 driver failed "
//		       "(errno = %d)\n", ret);
//	} else {
//		pr_info("Successfully added driver %s\n",
//			ft6x06_ts_driver.driver.name);
//	}
 	 printk("MediaTek FT5206 touch panel driver init\n");
	   i2c_register_board_info(TPD_I2C_NUMBER, &ft5206_i2c_tpd, 1);
		 if(tpd_driver_add(&tpd_device_driver) < 0)
			 TPD_DMESG("add FT5206 driver failed\n");
	 return 0;
}

static void __exit ft6x06_ts_exit(void)
{
	 TPD_DMESG("MediaTek FT5206 touch panel driver exit\n");
	 //input_unregister_device(tpd->dev);
	 tpd_driver_remove(&tpd_device_driver);
	//i2c_del_driver(&ft6x06_ts_driver);
}

module_init(ft6x06_ts_init);
module_exit(ft6x06_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft6x06 TouchScreen driver");
MODULE_LICENSE("GPL");
