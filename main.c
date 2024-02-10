#include <ctype.h>
#include <stdio.h>
#include "esp32c3.h"

#define BTN_PIN 9
#define LED_PIN 7
#define P(x) seg(#x, (void*)x)

void ir_generic(uint32_t e) {
  printf("Ulaz u interrupt rutinu INT %d maskA %x : %x : %x\n", e, CPU_INT_EIP_STATUS_REG, INTR_STATUS_0_REG, get_mstatus());

  switch (e) {
    case 2:
      // interrupt_example
      GPIO_STATUS_W1TC_REG = 1 << BTN_PIN; // Upisom 1 u ovaj reg, resetuje se odgovarajuci bit u statusnom registru
      break;
    case 3:
      // adc_example
      printf("APB_SARADC_INT_RAW_REG : %x\n", APB_SARADC_INT_RAW_REG);
      APB_SARADC_ONETIME_SAMPLE_REG = 0;
      APB_SARADC_INT_CLR_REG = 0xFFFFFFFF;
      printf("Ocitao sam %d %x\n", APB_SARADC_ADC1_DATA, APB_SARADC_INT_RAW_REG);
      break;
    case 4:
      // timer_example
      TIMG0_INT_CLR_TIMERS_REG = 1;
      TIMG0_T0CONFIG_REG |= 1ul << 10;
  }

  printf("Izlaz iz rutine INT %d maskB %x : %x : %x\n", e, CPU_INT_EIP_STATUS_REG, INTR_STATUS_0_REG, get_mstatus());
}


// Ispisuje 16x16 = 256 bajtova sa lokacije b
static void tabla(char* b) {
uint32_t i, j;

  for (i = 0; i < 16; i++) {
  	printf("%8X: ", (uintptr_t)b);
  	for (j = 0; j < 16; j++)
  	  printf("%3X", b[j]);
  	printf(" -- %X\n", 16*i);
  	b += 16;
  } 
}

// No comment
static void hello_example() {
  while(1) {
    printf("Hello\n");
    delay_ms(1000);
  }
}

// Ovo pali i gasi napon na GPIO pinu LED_PIN. Trenutno na ovaj pin nije zakaceno nista pa se to ne moze videti, osim merenjem napona.
static void blink_example() {
  GPIO_ENABLE_REG = 1 << LED_PIN; // Konfigurisemo GPIO pin na kome je LED na output funkciju
  while(1) {
    GPIO_OUT_REG = 1 << LED_PIN; // Postavljanjem odgovarajuceg bita, napon na GPIO, za koji je povezana LED postaje 3.3 V
    delay_ms(1000);
    GPIO_OUT_REG = 0; // Napon na svim GPIO postaje 0 V
    delay_ms(1000);
  }
}

// Reaguje na taster BOOT sa razvojne plocice
static void button_example() {
  GPIO_ENABLE_REG = 0; // Svi GPIO (pa i BTN_PIN) imaju funkciju input
  int p = 0, t;
  while(1) {
    t = (GPIO_IN_REG >> BTN_PIN) & 1; // Trenutno stanje na BTN_PIN pinu (0 je 0V, 1 je 3.3V)
    if (p != t) {
      printf("Novo stanje tastera je %d\n", t);
      p = t;
    }
  }
}

// Reaguje na taster BOOT
static void interrupt_example() {
  GPIO_ENABLE_REG = 0; // Svi GPIO (pa i BTN_PIN) imaju funkciju input
  INTERRUPT_CORE0_CPU_INT_TYPE_REG = 0; // Svi interapti su tipa level (nisu edge)
  INTERRUPT_CORE0_GPIO_INTERRUPT_PRO_MAP_REG = 2; // GPIO kontroler ce okidati IRQ 2
  INTERRUPT_CORE0_CPU_INT_PRI_n_REG[2] = 15; // Prioritet INT 2 je 15 (to je max)
  INTERRUPT_CORE0_CPU_INT_ENABLE_REG = 1 << 2; // Omoguceni su INT 0 i 2 (0 je implicitno uvek ukljucena, taj bit se zanemaruje)
  GPIO_PINn_REG[BTN_PIN] = (2 << 7) | (1 << 13); // Pin BTN_PIN ce geneisati interapt na silaznu ivicu i taj IRQ ce trajati jedan takt (edge), v. opis ovog registra
  while (1) { }
}

// Meri napon na pinu broj 2, posto je pull up trebalo bi da se dobija max 12-bitni broj, tj. 4095
// Nije proradilo
static void adc_example() {
  GPIO_ENABLE_REG = 0; // Svi GPIO imaju funkciju input
  INTERRUPT_CORE0_CPU_INT_TYPE_REG = 0; // Svi interapti su tipa level (nisu edge)
  INTERRUPT_CORE0_APB_ADC_INT_MAP_REG = 3; // ADC ide na IRQ 3
  INTERRUPT_CORE0_CPU_INT_PRI_n_REG[3] = 15; // Prioritet 15
  INTERRUPT_CORE0_CPU_INT_ENABLE_REG = 1 << 3;
  IO_MUX_GPIOn_REG[2] = (1u << 12) | (1u << 8);; // Funkcija 1 (analog), 8 je pullup
  SYSTEM_PERIP_CLK_EN0_REG |= 1 << 28; // Clock enable
  APB_SARADC_INT_ENA_REG = 0xffffffff; // ADC kontroleru dopusteno da podize IRQ za sta god hoce

  *((uint32_t*)0x60040000) = 0x580000C0; // Ovoga nema u DOC
  *((uint32_t*)0x60040054) = 0x40400F;

  APB_SARADC_CTRL2_REG = 1u << 24;
  APB_SARADC_ONETIME_SAMPLE_REG = (1ul < 31) | (1ul << 29) | (2u << 25); // kanal 2 = gpio2
  while(1) {
    delay_ms(1000);
    printf("Vreme %lld\n", get_ticks());
  }
}

// RAM banke
// #0 0x3fc80000; // Nizi deo IVT i program, zatim data i bss
// #1 0x3fcA0000; // Izgleda da je ROM ipak koristi
// #2 0x3fcC0000; // Najvisi deo koriste ROM rutine, malo nize je stek trenutno, nizi deo nije u upotrebi

// Inicijalizuje nulti KB banke 2
static void wrRAM() {
  uintptr_t i = 0x3fcC0000;
  for (; i < 0x3fcC0400; i +=4) {
    printf("Pisem %x\n", i);
    //delay_ms(100);
    *((uint32_t*)i) = i;
  }
}

// Proverava datu prvog KB banke 2
static void rdRAM() {
  uintptr_t i = 0x3fcC0000;
  for (; i < 0x3fcC0400; i +=4)
    if (*((uint32_t*)(i + 0x400)) != i) {
      printf("Ne slaze se na addr %x %x\n", i, *((uint32_t*)i));
      return;
  }
  printf("Slaze se sve do %x\n", i);
}

// Nije proradio
static void dma_example() {
  uint32_t desc[2][3];

  wrRAM(); // Inicijalizuje nulti KB banke 2
  rdRAM(); // Uverimo se da prvi KB banke 2 nije isti kao nulti

  // Ovo su dma deskriptori koji bi trebalo da opisu kopiranje nultog u prvi KB, banke 1
  // DMA
  // 0 -> 1
  desc[0][0] = (1ul << 31) | (1ul << 30) | (1024ul << 12) | 1024ul;
  desc[0][1] = 0x3fcC0000;
  desc[0][2] = 0;
  desc[1][0] = (1ul << 31) | 1024ul;
  desc[1][1] = 0x3fcC0400;
  desc[1][2] = 0;

  SYSTEM_PERIP_RST_EN1_REG &= ~0x40ul; // Reset DMA
  SYSTEM_PERIP_CLK_EN1_REG |= 0x40ul;  // Clock enable
  GDMA_OUT_CONF0_CH0_REG = 1;
  GDMA_OUT_CONF0_CH0_REG = 0;
  GDMA_IN_CONF0_CH0_REG = 1;
  GDMA_IN_CONF0_CH0_REG = 0;
  GDMA_OUT_LINK_CH0_REG = (uint32_t)desc[0];
  GDMA_IN_LINK_CH0_REG = (uint32_t)desc[1];
  GDMA_IN_CONF0_CH0_REG |= 0x10ul;
  GDMA_OUT_LINK_CH0_REG |= 0x200000ul;
  GDMA_IN_LINK_CH0_REG |= 0x400000ul;

  delay_ms(1000);

  // Sada bi trebalo da su nulti i prvi KB identicni
  rdRAM();

  printf("ROB BSS END: %X\n", _bss_end_ets);

  while (1){};
}

// Interapt na svakih sekund
static void timer_example() {
  TIMG0_T0CONFIG_REG = (3ul << 29) | (80ul << 13) | (1ul << 10); // clk / 80 = 1 MHz ????, napred, start, alarm, disabled
  INTERRUPT_CORE0_CPU_INT_TYPE_REG = 0; // Svi interapti su tipa level (nisu edge)
  INTERRUPT_CORE0_TG_T0_INT_MAP_REG = 4; // Timer0 na IRQ 4
  INTERRUPT_CORE0_CPU_INT_PRI_n_REG[4] = 15; // Prioritet 15
  INTERRUPT_CORE0_CPU_INT_ENABLE_REG = 1 << 4;
  TIMG0_T0LOADLO_REG = 0; // Pocetna vrednost
  TIMG0_T0LOADHI_REG = 0;
  TIMG0_T0LOAD_REG = 0;
  TIMG0_T0ALARMLO_REG = 1000000; // Postsclaer, 1Mhz -> 1 Hz
  TIMG0_T0ALARMHI_REG = 0;
  TIMG0_INT_ENA_TIMERS_REG = 1; // Enable interrupt
  TIMG0_T0CONFIG_REG |= 1ul << 31;
  while(1) {};
}

void start() {
  uint32_t p, t;
  uint32_t i;

  disable_wdt();

//  hello_example();
//  blink_example();
//  button_example();
//  interrupt_example();
//  adc_example();
//  dma_example();
  timer_example();

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
  printf("GPIO_INTERRUPT_PRO_MAP %x\n", INTERRUPT_CORE0_GPIO_INTERRUPT_PRO_MAP_REG);
  printf("CPU_INT_ENABLE_REG %x\n", CPU_INT_ENABLE_REG);
  printf("CPU_INT_TYPE_REG %x\n", CPU_INT_TYPE_REG);
      	p = t;
      }
  }
}
