#include <ctype.h>
#include <esp32c3.h>
#include <stdio.h>

#define BTN_PIN 9
#define LED_PIN 7
#define P(x) seg(#x, (void*)x)

void ir_generic(uint32_t e) {
  printf("INT %d maskA %x : %x : %x\n", e, CPU_INT_EIP_STATUS_REG, INTR_STATUS_0_REG, get_mstatus());
  GPIO_STATUS_W1TC_REG = 1 << BTN_PIN;
//  GPIO_STATUS_W1TC = 0;
  // CPU_INT_CLEAR_REG = 4; //int 2
  if (e == 5)
    UART_INT_CLR_REG = 0xffffffff;
  if (e == 3) {
    printf("APB_SARADC_INT_RAW_REG : %x\n", APB_SARADC_INT_RAW_REG);
    APB_SARADC_ONETIME_SAMPLE_REG = 0;
    APB_SARADC_INT_CLR_REG = 0xFFFFFFFF;
    CPU_INT_CLEAR_REG = 1 << 3;
    asm volatile("fence\n");
    printf("Ocitao sam %d %x\n", APB_SARADC_ADC1_DATA, APB_SARADC_INT_RAW_REG);

  }
  printf("INT %d maskB %x : %x : %x\n", e, CPU_INT_EIP_STATUS_REG, INTR_STATUS_0_REG, get_mstatus());
}

void tabla(char* b) {
uint32_t i, j;

  for (i = 0; i < 16; i++) {
  	printf("%8X: ", (uintptr_t)b);
  	for (j = 0; j < 16; j++)
  	  printf("%3X", b[j]);
  	printf(" -- %X\n", 16*i);
  	b += 16;
  } 
}

// RAM banke
// 0x3fc80000;
// 0x3fcA0000;
// 0x3fcC0000;

void wrRAM() {
  uintptr_t i = 0x3fcA0000;
  for (; i < 0x3fcA0400; i +=4) {
  	printf("Pisem %x\n", i);
  	*((uint32_t*)i) = i;
  }
}

void rdRAM() {
  uintptr_t i = 0x3fcA0400;
  for (; i < 0x3fcA0800; i +=4)
  	if (*((uint32_t*)i) + 0x400 != i) {
  	printf("Ne slaze se na addr %x %x\n", i, *((uint32_t*)i));
  	return;
  }
  printf("Slaze se sve do %x\n", i);
}

void start(uint32_t a0) {
  uint32_t p, t;
  uint32_t i;
  uint32_t desc[2][3];

  //disable_wdt();

  TRIS = 1 << LED_PIN;

  CPU_INT_TYPE_REG = 0;// level 0xffffffff; // edge
  INTERRUPT_CORE0_GPIO_INTERRUPT_PRO_MAP_REG = 2; // kacimo se na dvojku
  INTERRUPT_CORE0_CPU_INT_PRI_n_REG[2] = 15; // prioritet dvojke
  INTERRUPT_CORE0_CPU_INT_PRI_n_REG[3] = 15; // prio trojke
  CPU_INT_ENABLE_REG = 0xffffffff;//1 << 2;

  PIN_REG[BTN_PIN] = (2 << 7) | (1 << 13); // falling edge trigger

  printf("SP: %x\n", a0);

  IO_MUX_GPIOn_REG[2] = (1u << 12) | (1u << 8);; // Funkcija 1, 8 je pullup
  TRIS = 0;//1 << 2;

  APB_SARADC_INT_ENA_REG = 0xffffffff;

printf("Oh kakav register %X\n", SYSTEM_PERIP_CLK_EN0_REG);
printf("Interapt za ADC %d\n", INTERRUPT_CORE0_APB_ADC_INT_MAP_REG);
INTERRUPT_CORE0_APB_ADC_INT_MAP_REG = 3; // adc kacimo na 3
*((uint32_t*)0x60040000) = 0x580000C0;
*((uint32_t*)0x60040054) = 0x40400F;
    APB_SARADC_CTRL2_REG = 1u << 24;
    APB_SARADC_ONETIME_SAMPLE_REG =  /*0x24000000;//*/  (1ul < 31) | (1ul << 29) | (2u << 25); // kanal 2 = gpio2
  while(1) {
    //*((uint32_t*)0x60040010) = 0x20890000;

    feed_wdt();
    delay_ms(1000);

    printf("Vreme %lld\n", get_ticks());
  }

  for (i = 0; i < 4; i++)
    printf("APB_CTRL_FLASH_ACE_ADDR_REG[%d] = %x sz: %x\n", i, APB_CTRL_FLASH_ACE_ADDR_REG[i], APB_CTRL_FLASH_ACE_SIZE_REG[i]);

  for (i = 0; i < 4; i++) {
    // APB_CTRL_FLASH_ACE_ADDR_REG[i] = 0x100000 * i;
    // APB_CTRL_FLASH_ACE_SIZE_REG[i] = 0x100; //10 ili 100 predstavlja 1M, nije jasno
    APB_CTRL_FLASH_ACE_ATTR[i] = 0;//0xf; // SPI & CACHE = rwx
  }

  for (i = 0; i < 4; i++)
    printf("APB_CTRL_FLASH_ACE_ADDR_REG[%d] = %x sz: %x\n", i, APB_CTRL_FLASH_ACE_ADDR_REG[i], APB_CTRL_FLASH_ACE_SIZE_REG[i]);

  printf("EXTMEM_IBUS_PMS_TBL_LOCK_REG: %x\n", EXTMEM_IBUS_PMS_TBL_LOCK_REG);
  printf("EXTMEM_DBUS_PMS_TBL_LOCK_REG: %x\n", EXTMEM_DBUS_PMS_TBL_LOCK_REG);

  // EXTMEM_IBUS_PMS_TBL_LOCK_REG = 1;
  // EXTMEM_DBUS_PMS_TBL_LOCK_REG = 1;

  wrRAM();
  rdRAM();

// DMA
// 0 -> 1
desc[0][0] = (1ul << 31) | (1ul << 30) | (1024ul << 12) | 1024ul;
desc[0][1] = 0x3fcA0000;
desc[0][2] = 0;
desc[1][0] = (1ul << 31) | 1024ul;
desc[1][1] = 0x3fcA0400;
desc[1][2] = 0;

SYSTEM_PERIP_CLK_EN1_REG |= 0x40ul;
SYSTEM_PERIP_RST_EN1_REG &= ~0x40ul;
GDMA_OUT_CONF0_CH0 = 1;
GDMA_OUT_CONF0_CH0 = 0;
GDMA_IN_CONF0_CH0 = 1;
GDMA_IN_CONF0_CH0 = 0;
GDMA_OUT_LINK_CH0 = (uint32_t)desc[0];
GDMA_IN_LINK_CH0 = (uint32_t)desc[1];
GDMA_IN_CONF0_CH0 |= 0x10ul;
GDMA_OUT_LINK_CH0 |= 0x200000ul;
GDMA_IN_LINK_CH0 |= 0x400000ul;

delay_ms(1000);
rdRAM();

// printf("EXTMEM_IBUS_PMS_TBL_LOCK_REG: %x\n", EXTMEM_IBUS_PMS_TBL_LOCK_REG);
// printf("EXTMEM_DBUS_PMS_TBL_LOCK_REG: %x\n", EXTMEM_DBUS_PMS_TBL_LOCK_REG);

for (i = 0; i < 128; i++) MMU[i] = i;

// Prva 4 MB su region 0
EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[0] = 0;
EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[1] = 0x400;
EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[2] = 0x400;
EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[0] = 0;
EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[1] = 0x400;
EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[2] = 0x400;

// for (i = 0; i < 3; i++) {
//   EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[i] = 3 * i + 1;
//  EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[i] = i;
// }

  EXTMEM_IBUS_PMS_TBL_ATTR_REG = 0xff; // region 1 i 2, rx u priv i nonpriv
  EXTMEM_DBUS_PMS_TBL_ATTR_REG = 0xf; // region 1 i 2, r u priv i nonpriv

  printf("EXTMEM_IBUS_PMS_TBL_ATTR_REG: %x\n", EXTMEM_IBUS_PMS_TBL_ATTR_REG);
  printf("EXTMEM_DBUS_PMS_TBL_ATTR_REG: %x\n", EXTMEM_DBUS_PMS_TBL_ATTR_REG);

  for (i = 0; i < 3; i++) {
    printf("EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[%d] = %x\n", i, EXTMEM_DBUS_PMS_TBL_BOUNDARY_REG[i]);
    printf("EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[%d] = %x\n", i, EXTMEM_IBUS_PMS_TBL_BOUNDARY_REG[i]);

}

  int x = *((int*)0x3c000100);
  printf("KukocVNA %d\n", x);
  tabla((char*)0x3c001000);

  char* b = (char*)0x3c000000;
  for (i = 0; i < 1024 * 1024 * 8; i++) 
    if (b[i]) { printf("Nasao %x\n", i); break ;}

  p = 0;
  for (;;) {
    t = I & (1 << BTN_PIN);
      if (p != t) {
      	printf("LED: %x mstatus: %x\n", t, get_mstatus());
  printf("GPIO_INTERRUPT_PRO_MAP %x\n", GPIO_INTERRUPT_PRO_MAP);
  printf("CPU_INT_ENABLE_REG %x\n", CPU_INT_ENABLE_REG);
  printf("CPU_INT_TYPE_REG %x\n", CPU_INT_TYPE_REG);
      	p = t;
      }
  }
}
