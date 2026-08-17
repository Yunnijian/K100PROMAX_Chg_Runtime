/* SPDX-License-Identifier: (GPL-2.0-only OR MIT) */


#ifndef _DT_BINDINGS_AMLOGIC_C3_RESET_H
#define _DT_BINDINGS_AMLOGIC_C3_RESET_H



#define RESET_USBCTRL				4

#define RESET_USBPHY20				8

#define RESET_USB2DRD				10
#define RESET_MIPI_DSI_HOST			11
#define RESET_MIPI_DSI_PHY			12

#define RESET_GE2D				21
#define RESET_DWAP				22



#define RESET_AUDIO				32

#define RESET_DDRAPB				35
#define RESET_DDR				36
#define RESET_DOS_CAPB3				37
#define RESET_DOS				38

#define RESET_NNA				47
#define RESET_ETHERNET				48
#define RESET_ISP				49
#define RESET_VC9000E_APB			50
#define RESET_VC9000E_A				51

#define RESET_VC9000E_CORE			53



#define RESET_ABUS_ARB				64
#define RESET_IRCTRL				65

#define RESET_TEMP_PII				67

#define RESET_SPICC_0				73
#define RESET_SPICC_1				74
#define RESET_RSA				75


#define RESET_MSR_CLK				80
#define RESET_SPIFC				81
#define RESET_SAR_ADC				82

#define RESET_ACODEC				88

#define RESET_WATCHDOG				91



#define RESET_ISP_NIC_GPV			96
#define RESET_ISP_NIC_MAIN			97
#define RESET_ISP_NIC_VCLK			98
#define RESET_ISP_NIC_VOUT			99
#define RESET_ISP_NIC_ALL			100
#define RESET_VOUT				101
#define RESET_VOUT_VENC				102

#define RESET_CVE_NIC_GPV			104
#define RESET_CVE_NIC_MAIN			105
#define RESET_CVE_NIC_GE2D			106
#define RESET_CVE_NIC_DW			106
#define RESET_CVE_NIC_CVE			108
#define RESET_CVE_NIC_ALL			109
#define RESET_CVE				110



#define RESET_RTC				128
#define RESET_PWM_AB				129
#define RESET_PWM_CD				130
#define RESET_PWM_EF				131
#define RESET_PWM_GH				132
#define RESET_PWM_IJ				133
#define RESET_PWM_KL				134
#define RESET_PWM_MN				135

#define RESET_UART_A				138
#define RESET_UART_B				139
#define RESET_UART_C				140
#define RESET_UART_D				141
#define RESET_UART_E				142
#define RESET_UART_F				143
#define RESET_I2C_S_A				144
#define RESET_I2C_M_A				145
#define RESET_I2C_M_B				146
#define RESET_I2C_M_C				147
#define RESET_I2C_M_D				148

#define RESET_SD_EMMC_A				152
#define RESET_SD_EMMC_B				153
#define RESET_SD_EMMC_C				154



#define RESET_BRG_NIC_NNA			173
#define RESET_BRG_MUX_NIC_MAIN			174
#define RESET_BRG_AO_NIC_ALL			175

#define RESET_BRG_NIC_VAPB			184
#define RESET_BRG_NIC_SDIO_B			185
#define RESET_BRG_NIC_SDIO_A			186
#define RESET_BRG_NIC_EMMC			187
#define RESET_BRG_NIC_DSU			188
#define RESET_BRG_NIC_SYSCLK			189
#define RESET_BRG_NIC_MAIN			190
#define RESET_BRG_NIC_ALL			191

#endif
