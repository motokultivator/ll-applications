#include <stdint.h>

// https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf
extern volatile uint32_t O;
extern volatile uint32_t SO;
extern volatile uint32_t CO;
extern volatile uint32_t TRIS;
extern volatile uint32_t OTRIS;
extern volatile uint32_t CTRIS;
extern volatile uint32_t STRAP;
extern volatile uint32_t I;
extern volatile uint32_t S;
extern volatile uint32_t SS;
extern volatile uint32_t CS;
extern volatile uint32_t PIN_REG[22];  // GPIO_PINn_REG, page 177
extern volatile uint32_t FUNCIN[128];
extern volatile uint32_t FUNCOUT[22];
extern volatile uint32_t IO_MUX_GPIO[22];
extern volatile uint32_t GPIO_INTERRUPT_PRO_MAP;
extern volatile uint32_t CPU_INT_ENABLE_REG;
extern volatile uint32_t CPU_INT_TYPE_REG;
extern volatile uint32_t CPU_INT_PRI[32];
extern volatile uint32_t CPU_INT_CLEAR_REG;
extern volatile uint32_t INTR_STATUS_0_REG;
extern volatile uint32_t INTR_STATUS_1_REG;
extern volatile uint32_t CPU_INT_EIP_STATUS_REG;
extern volatile uint32_t GPIO_STATUS_W1TC;
extern volatile uint32_t APB_CTRL_FLASH_ACE_ADDR_REG[4];
extern volatile uint32_t APB_CTRL_FLASH_ACE_SIZE_REG[4];
extern volatile uint32_t APB_CTRL_FLASH_ACE_ATTR[4];
extern volatile uint32_t EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[3]; // x 0x1000
extern volatile uint32_t EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[3];
extern volatile uint32_t EXTMEM_IBUS_PMS_TBL_ATTR_REG;
extern volatile uint32_t EXTMEM_DBUS_PMS_TBL_ATTR_REG;
extern volatile uint32_t EXTMEM_DBUS_PMS_TBL_LOCK_REG;
extern volatile uint32_t EXTMEM_IBUS_PMS_TBL_LOCK_REG;
extern volatile uint32_t PMS_INTERNAL_SRAM_USAGE_CPU_CACHE;
extern volatile uint32_t MMU[128];

extern volatile uint32_t SYSTEM_PERIP_CLK_EN1_REG;
extern volatile uint32_t SYSTEM_PERIP_RST_EN1_REG;
extern volatile uint32_t GDMA_OUT_CONF0_CH0;
extern volatile uint32_t GDMA_IN_CONF0_CH0;
extern volatile uint32_t GDMA_OUT_LINK_CH0;
extern volatile uint32_t GDMA_IN_LINK_CH0;

extern volatile uint32_t APB_SARADC_ONETIME_SAMPLE_REG;
extern volatile uint32_t APB_SARADC_ADC1_DATA;
extern volatile uint32_t APB_SARADC_CTRL2_REG;

extern volatile uint32_t SYSTIMER_UNIT0_OP_REG;
extern volatile uint32_t SYSTIMER_UNIT0_VALUE[2];

extern volatile uint32_t RTC_CNTL_DIG_ISO_REG;
extern volatile uint32_t RTC_CNTL_WDTCONFIG0_REG;
extern volatile uint32_t RTC_CNTL_WDT_FEED;
extern volatile uint32_t RTC_CNTL_WDTWPROTECT_REG;
extern volatile uint32_t RTC_CNTL_SWD_CONF_REG;
extern volatile uint32_t RTC_CNTL_SWD_WPROTECT_REG;
extern volatile uint32_t RTC_CNTL_SW_CPU_STALL_REG;

extern volatile uint32_t TIMG0_WDTCONFIG0_REG;
extern volatile uint32_t TIMG0_REGCLK_REG;
extern volatile uint32_t TIMG0_WDTCONFIG0_REG;
extern volatile uint32_t TIMG0_REGCLK_REG;

uint32_t get_mstatus();
uintptr_t get_sp();
uint64_t get_ticks();
void delay_us(uint32_t us);
void delay_ms(uint32_t us);

static inline void disable_wdt(void) {
  RTC_CNTL_WDTWPROTECT_REG = 0x50d83aa1;
  RTC_CNTL_DIG_ISO_REG = 0;
  RTC_CNTL_WDTCONFIG0_REG = 0;
  RTC_CNTL_WDTWPROTECT_REG = 0;
}

static inline void feed_wdt(void) {
  RTC_CNTL_WDTWPROTECT_REG = 0x50d83aa1;
  RTC_CNTL_WDT_FEED = 0x80000000;
  RTC_CNTL_WDTWPROTECT_REG = 0;
}
