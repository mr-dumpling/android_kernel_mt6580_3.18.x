/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include <linux/regulator/consumer.h>

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(PFX fmt, ##arg)

/*#define DEBUG_CAMERA_HW_K*/
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)   pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
        do {    \
           pr_debug(PFX fmt, ##arg); \
        } while (0)
#else
#define PK_DBG(a, ...)
#define PK_ERR(a, ...)
#define PK_XLOG_INFO(fmt, args...)
#endif

/* GPIO Pin control*/
struct platform_device *cam_plt_dev = NULL;
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam0_pnd_h = NULL;
struct pinctrl_state *cam0_pnd_l = NULL;
struct pinctrl_state *cam0_rst_h = NULL;
struct pinctrl_state *cam0_rst_l = NULL;
struct pinctrl_state *cam1_pnd_h = NULL;
struct pinctrl_state *cam1_pnd_l = NULL;
struct pinctrl_state *cam1_rst_h = NULL;
struct pinctrl_state *cam1_rst_l = NULL;
struct pinctrl_state *cam_ldo0_h = NULL;
struct pinctrl_state *cam_ldo0_l = NULL;

/*###############################################################################################################*/

int mtkcam_gpio_init(struct platform_device *pdev)
{
    int ret = 0;

    camctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR(camctrl)) {
        dev_err(&pdev->dev, "Cannot find camera pinctrl!");
        ret = PTR_ERR(camctrl);
    }

    /*Cam0 Power/Rst Ping initialization */
    cam0_pnd_h = pinctrl_lookup_state(camctrl, "cam0_pnd1");
    if (IS_ERR(cam0_pnd_h)) {
        ret = PTR_ERR(cam0_pnd_h);
        pr_debug("%s : pinctrl err, cam0_pnd_h\n", __func__);
    }

    cam0_pnd_l = pinctrl_lookup_state(camctrl, "cam0_pnd0");
    if (IS_ERR(cam0_pnd_l)) {
        ret = PTR_ERR(cam0_pnd_l);
        pr_debug("%s : pinctrl err, cam0_pnd_l\n", __func__);
    }

    cam0_rst_h = pinctrl_lookup_state(camctrl, "cam0_rst1");
    if (IS_ERR(cam0_rst_h)) {
        ret = PTR_ERR(cam0_rst_h);
        pr_debug("%s : pinctrl err, cam0_rst_h\n", __func__);
    }

    cam0_rst_l = pinctrl_lookup_state(camctrl, "cam0_rst0");
    if (IS_ERR(cam0_rst_l)) {
        ret = PTR_ERR(cam0_rst_l);
        pr_debug("%s : pinctrl err, cam0_rst_l\n", __func__);
    }

    /*Cam1 Power/Rst Ping initialization */
    cam1_pnd_h = pinctrl_lookup_state(camctrl, "cam1_pnd1");
    if (IS_ERR(cam1_pnd_h)) {
        ret = PTR_ERR(cam1_pnd_h);
        pr_debug("%s : pinctrl err, cam1_pnd_h\n", __func__);
    }

    cam1_pnd_l = pinctrl_lookup_state(camctrl, "cam1_pnd0");
    if (IS_ERR(cam1_pnd_l )) {
        ret = PTR_ERR(cam1_pnd_l );
        pr_debug("%s : pinctrl err, cam1_pnd_l\n", __func__);
    }

    cam1_rst_h = pinctrl_lookup_state(camctrl, "cam1_rst1");
    if (IS_ERR(cam1_rst_h)) {
        ret = PTR_ERR(cam1_rst_h);
        pr_debug("%s : pinctrl err, cam1_rst_h\n", __func__);
    }

    cam1_rst_l = pinctrl_lookup_state(camctrl, "cam1_rst0");
    if (IS_ERR(cam1_rst_l)) {
        ret = PTR_ERR(cam1_rst_l);
        pr_debug("%s : pinctrl err, cam1_rst_l\n", __func__);
    }

    /*externel LDO enable */
    cam_ldo0_h = pinctrl_lookup_state(camctrl, "cam_ldo0_1");
    if (IS_ERR(cam_ldo0_h)) {
        ret = PTR_ERR(cam_ldo0_h);
        pr_debug("%s : pinctrl err, cam_ldo0_h\n", __func__);
    }

    cam_ldo0_l = pinctrl_lookup_state(camctrl, "cam_ldo0_0");
    if (IS_ERR(cam_ldo0_l)) {
        ret = PTR_ERR(cam_ldo0_l);
        pr_debug("%s : pinctrl err, cam_ldo0_l\n", __func__);
    }
    return ret;
}

/*###############################################################################################################*/

int mtkcam_gpio_set(int PinIdx, int PwrType, int Val)
{
    int ret = 0;

    switch (PwrType) {
    case CAMRST:
        if (PinIdx == 0) {
            if (Val == 0)
                pinctrl_select_state(camctrl, cam0_rst_l);
            else
                pinctrl_select_state(camctrl, cam0_rst_h);
        } else {
            if (Val == 0)
                pinctrl_select_state(camctrl, cam1_rst_l);
            else
                pinctrl_select_state(camctrl, cam1_rst_h);
        }
        break;
    case CAMPDN:
        if (PinIdx == 0) {
            if (Val == 0)
                pinctrl_select_state(camctrl, cam0_pnd_l);
            else
                pinctrl_select_state(camctrl, cam0_pnd_h);
        } else {
            if (Val == 0)
                pinctrl_select_state(camctrl, cam1_pnd_l);
            else
                pinctrl_select_state(camctrl, cam1_pnd_h);
        }
        break;
    case CAMLDO:
        if (Val == 0)
            pinctrl_select_state(camctrl, cam_ldo0_l);
        else
            pinctrl_select_state(camctrl, cam_ldo0_h);
        break;
    default:
        PK_DBG("PwrType(%d) is invalid !!\n", PwrType);
        break;
    };

    PK_DBG("PinIdx(%d) PwrType(%d) val(%d)\n", PinIdx, PwrType, Val);

    return ret;
}

/*###############################################################################################################*/

int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On,
               char *mode_name)
{
    u32 pinSetIdx = 0;    /* default main sensor */

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3
#define VOL_2800 2800000
#define VOL_1800 1800000
#define VOL_1500 1500000
#define VOL_1200 1200000
#define VOL_1000 1000000

    u32 pinSet[3][8] = {

        /* for main sensor */
        {
        /* The reset pin of main sensor uses GPIO10 of mt6306, please call mt6306 API to set */
         CAMERA_CMRST_PIN,
         CAMERA_CMRST_PIN_M_GPIO,    /* mode */
         GPIO_OUT_ONE,    /* ON state */
         GPIO_OUT_ZERO,    /* OFF state */
         CAMERA_CMPDN_PIN,
         CAMERA_CMPDN_PIN_M_GPIO,
         GPIO_OUT_ONE,
         GPIO_OUT_ZERO,
         },
        /* for sub sensor */
        {
         CAMERA_CMRST1_PIN,
         CAMERA_CMRST1_PIN_M_GPIO,
         GPIO_OUT_ONE,
         GPIO_OUT_ZERO,
         CAMERA_CMPDN1_PIN,
         CAMERA_CMPDN1_PIN_M_GPIO,
         GPIO_OUT_ONE,
         GPIO_OUT_ZERO,
         },
        /* for main_2 sensor */
        {
         GPIO_CAMERA_INVALID,
         GPIO_CAMERA_INVALID,    /* mode */
         GPIO_OUT_ONE,    /* ON state */
         GPIO_OUT_ZERO,    /* OFF state */
         GPIO_CAMERA_INVALID,
         GPIO_CAMERA_INVALID,
         GPIO_OUT_ONE,
         GPIO_OUT_ZERO,
         }
    };

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx) {
        pinSetIdx = 0;
    } else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    } else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }

    /* power ON */
    if (On) {

        if (pinSetIdx == 0) {
            ISP_MCLK1_EN(1);
        } else if (pinSetIdx == 1) {
            ISP_MCLK2_EN(1);
        }

        PK_DBG("[PowerON]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);

        if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName))) {
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
            /* VCAM_IO */
            if (TRUE != _hwPowerOn(VCAMIO, VOL_1800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n",
                     VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_A */
            if (TRUE != _hwPowerOn(VCAMA, VOL_2800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n",
                     VCAMA);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_D_1200 */
            if (pinSetIdx == 0 && TRUE != _hwPowerOn(VCAMD, VOL_1200)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAMD), power id = %d\n",
                     VCAMD);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_AF */
            if (TRUE != _hwPowerOn(VCAMAF, VOL_2800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                     VCAMAF);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(5);
            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]);
            mdelay(1);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]);
        } else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2755_MIPI_RAW, currSensorName))) {
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
            mdelay(50);  
            /* VCAM_IO */
            if (TRUE != _hwPowerOn(VCAMIO, VOL_1800)) {
                PK_ERR
                    ("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n",
                     VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);
            /* VCAM_D_SUB */
            if (TRUE != _hwPowerOn(SUB_VCAMD, VOL_1800)) {
                PK_ERR
                    ("[CAMERA SENSOR] Fail to enable analog power (SUB_VCAM_D), power id = %d\n",
                     SUB_VCAMD);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(10);
            /* VCAM_A */
            if (TRUE != _hwPowerOn(VCAMA, VOL_2800)) {
                PK_ERR
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n",
                     VCAMA);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(20);
            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]);
            mdelay(5);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]);
        } else {
            /* First Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
            /* VCAM_IO */
            if (TRUE != _hwPowerOn(VCAMIO, VOL_1800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable IO power (VCAM_IO), power id = %d\n",
                     VCAMIO);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_A */
            if (TRUE != _hwPowerOn(VCAMA, VOL_2800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n",
                     VCAMA);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_D */
            if (pinSetIdx == 0 && TRUE != _hwPowerOn(VCAMD, VOL_1500)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAMD), power id = %d\n",
                     VCAMD);
                goto _kdCISModulePowerOn_exit_;
            }
            /* VCAM_AF */
            if (TRUE != _hwPowerOn(VCAMAF, VOL_2800)) {
                PK_DBG
                    ("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF),power id = %d\n",
                     VCAMAF);
                goto _kdCISModulePowerOn_exit_;
            }
            mdelay(5);
            /* enable active sensor */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN])
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_ON]);
            mdelay(1);
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_ON]);
        }

    } else {
    /* power OFF */

        if (pinSetIdx == 0) {
            ISP_MCLK1_EN(0);
        } else if (pinSetIdx == 1) {
            ISP_MCLK2_EN(0);
        }

        PK_DBG("[PowerOFF]pinSetIdx:%d\n", pinSetIdx);

            /* Set Power Pin low and Reset Pin Low */
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                mtkcam_gpio_set(pinSetIdx, CAMPDN,
                        pinSet[pinSetIdx][IDX_PS_CMPDN + IDX_PS_OFF]);
            }
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                mtkcam_gpio_set(pinSetIdx, CAMRST,
                        pinSet[pinSetIdx][IDX_PS_CMRST + IDX_PS_OFF]);
            }
                    if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName))) {
                            /* VCAM_D pinset */
                            if(pinSetIdx == 0 && TRUE != _hwPowerDown(VCAMD)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                                     VCAMD);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_A */
                            if (TRUE != _hwPowerDown(VCAMA)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                                     VCAMA);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_IO */
                            if (TRUE != _hwPowerDown(VCAMIO)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d\n",
                                     VCAMIO);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* AF_VCC */
                            if (TRUE != _hwPowerDown(VCAMAF)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                                     VCAMAF);
                                goto _kdCISModulePowerOn_exit_;
                            }
                    } else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2755_MIPI_RAW, currSensorName))) {
                            /* VCAM_D_SUB */
                            if (TRUE != _hwPowerDown(SUB_VCAMD)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF analog power (SUB_VCAM_D),power id= (%d)\n",
                                     SUB_VCAMD);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_A */
                            if (TRUE != _hwPowerDown(VCAMA)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                                     VCAMA);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_IO */
                            if (TRUE != _hwPowerDown(VCAMIO)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d\n",
                                     VCAMIO);
                                goto _kdCISModulePowerOn_exit_;
                            }
                    } else {
                            /* VCAM_D_SUB */
                            if (TRUE != _hwPowerDown(SUB_VCAMD)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF analog power (SUB_VCAM_D),power id= (%d)\n",
                                     SUB_VCAMD);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_D */
                            if (TRUE != _hwPowerDown(VCAMD)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF core power (VCAM_D),power id = %d\n",
                                     VCAMD);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_A */
                            if (TRUE != _hwPowerDown(VCAMA)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A),power id= (%d)\n",
                                     VCAMA);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* VCAM_IO */
                            if (TRUE != _hwPowerDown(VCAMIO)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d\n",
                                     VCAMIO);
                                goto _kdCISModulePowerOn_exit_;
                            }
                            /* AF_VCC */
                            if (TRUE != _hwPowerDown(VCAMAF)) {
                                PK_DBG
                                    ("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF),power id = %d\n",
                                     VCAMAF);
                                goto _kdCISModulePowerOn_exit_;
                            }
                    }
    }

    return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

/*###############################################################################################################*/

EXPORT_SYMBOL(kdCISModulePowerOn);

