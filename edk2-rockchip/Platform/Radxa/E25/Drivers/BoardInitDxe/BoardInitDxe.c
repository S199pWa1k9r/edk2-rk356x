/** @file
 *
 *  Board init for the RADXA E25 platform
 *
 *  Copyright (c) 2021, Jared McNeill <jmcneill@invisible.ca>
 *  Copyright (c) 2023, Sergey Tyuryukanov <s199p.wa1k9r@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/CruLib.h>
#include <Library/GpioLib.h>
#include <Library/I2cLib.h>
#include <Library/MultiPhyLib.h>
#include <Library/OtpLib.h>
#include <Library/SocLib.h>
#include <Library/Pcie30PhyLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

#include <IndustryStandard/Rk356x.h>
#include <IndustryStandard/Rk356xCru.h>

/*
 * PMIC registers
*/
#define PMIC_I2C_ADDR           0x20

#define PMIC_CHIP_NAME          0xed
#define PMIC_CHIP_VER           0xee
#define PMIC_POWER_EN1          0xb2
#define PMIC_POWER_EN2          0xb3
#define PMIC_POWER_EN3          0xb4
#define PMIC_LDO1_ON_VSEL       0xcc
#define PMIC_LDO2_ON_VSEL       0xce
#define PMIC_LDO3_ON_VSEL       0xd0
#define PMIC_LDO4_ON_VSEL       0xd2
#define PMIC_LDO6_ON_VSEL       0xd6
#define PMIC_LDO7_ON_VSEL       0xd8
#define PMIC_LDO8_ON_VSEL       0xda
#define PMIC_LDO9_ON_VSEL       0xdc

/*
 * CPU_GRF registers
*/
#define GRF_CPU_COREPVTPLL_CON0               (CPU_GRF + 0x0010)
#define  CORE_PVTPLL_RING_LENGTH_SEL_SHIFT    3
#define  CORE_PVTPLL_RING_LENGTH_SEL_MASK     (0x1FU << CORE_PVTPLL_RING_LENGTH_SEL_SHIFT)
#define  CORE_PVTPLL_OSC_EN                   BIT1
#define  CORE_PVTPLL_START                    BIT0

/*
 * SYS_GRF registers
 */
#define GRF_IOFUNC_SEL0                       (SYS_GRF + 0x0300)
#define GRF_IOFUNC_SEL5                       (SYS_GRF + 0x0314)
#define  PCIE20X1_IOMUX_SEL_MASK              (BIT3|BIT2)
#define  PCIE20X1_IOMUX_SEL_M1                BIT2
#define  PCIE20X1_IOMUX_SEL_M2                BIT3
#define  PCIE30X1_IOMUX_SEL_MASK              (BIT5|BIT4)
#define  PCIE30X1_IOMUX_SEL_M1                BIT4
#define  PCIE30X1_IOMUX_SEL_M2                BIT5
#define  PCIE30X2_IOMUX_SEL_MASK              (BIT7|BIT6)
#define  PCIE30X2_IOMUX_SEL_M1                BIT6
#define  PCIE30X2_IOMUX_SEL_M2                BIT7


/*
 * PMU registers
 */
#define PMU_NOC_AUTO_CON0                     (PMU_BASE + 0x0070)
#define PMU_NOC_AUTO_CON1                     (PMU_BASE + 0x0074)

STATIC CONST GPIO_IOMUX_CONFIG mSdmmc1IomuxConfig[] = {
  { "sdmmc1_d0",          2, GPIO_PIN_PA3, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "sdmmc1_d1",          2, GPIO_PIN_PA4, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "sdmmc1_d2",          2, GPIO_PIN_PA5, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "sdmmc1_d3",          2, GPIO_PIN_PA6, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "sdmmc1_cmd",         2, GPIO_PIN_PA7, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "sdmmc1_clk",         2, GPIO_PIN_PB0, 1, GPIO_PIN_PULL_UP,   GPIO_PIN_DRIVE_2 },
  { "wifi_reg_on",        2, GPIO_PIN_PB7, 0, GPIO_PIN_PULL_NONE, GPIO_PIN_DRIVE_DEFAULT },
};

STATIC CONST GPIO_IOMUX_CONFIG mPci20x1IomuxConfig[] = {
  { "pcie20_clkreqnm2",   1, GPIO_PIN_PB0, 4, GPIO_PIN_PULL_NONE, GPIO_PIN_DRIVE_DEFAULT },
};

STATIC CONST GPIO_IOMUX_CONFIG mPcie30x1IomuxConfig[] = {
};

STATIC CONST GPIO_IOMUX_CONFIG mPcie30x2IomuxConfig[] = {
  { "pcie30x2_clkreqnm1", 2, GPIO_PIN_PD4, 4, GPIO_PIN_PULL_NONE, GPIO_PIN_DRIVE_DEFAULT },
  { "pcie30x2_perstnm1",  2, GPIO_PIN_PD6, 4, GPIO_PIN_PULL_NONE, GPIO_PIN_DRIVE_DEFAULT },
  { "pcie30x2_wakenm1",   2, GPIO_PIN_PD5, 4, GPIO_PIN_PULL_NONE, GPIO_PIN_DRIVE_DEFAULT },
};

STATIC
VOID
BoardInitPcie (
  VOID
  )
{

#if (PCIE2x1_ENABLE  == 1)
  DEBUG ((DEBUG_INFO, "PCIe: INIT PCIE2x1\n"));

  /* Configure PINs */
  MmioWrite32 (GRF_IOFUNC_SEL5, (PCIE20X1_IOMUX_SEL_MASK << 16) | PCIE20X1_IOMUX_SEL_M2);
  GpioSetIomuxConfig (mPci20x1IomuxConfig, ARRAY_SIZE (mPci20x1IomuxConfig));

  /* PCIe power UP gpio = <&gpio0 RK_PC7 GPIO_ACTIVE_HIGH>; */
  //GpioPinSetDirection (0, GPIO_PIN_PC7, GPIO_PIN_OUTPUT);
  //GpioPinWrite (0, GPIO_PIN_PC7, TRUE);

  //GpioPinSetDirection (3, GPIO_PIN_PA7, GPIO_PIN_OUTPUT);
  //GpioPinWrite (3, GPIO_PIN_PA7, TRUE);
#endif

#if (PCIE3x1_ENABLE  == 1)
  DEBUG ((DEBUG_INFO, "PCIe: INIT PCIE3x1\n"));
#endif

#if (PCIE3x2_ENABLE == 1)
  DEBUG ((DEBUG_INFO, "PCIe: INIT PCIE3x2\n"));
  GpioSetIomuxConfig (mPcie30x2IomuxConfig, ARRAY_SIZE (mPcie30x2IomuxConfig));

  /* PCIe30x2 IO mux selection - M1 */
  MmioWrite32 (GRF_IOFUNC_SEL5, (PCIE30X2_IOMUX_SEL_MASK << 16) | PCIE30X2_IOMUX_SEL_M1);

  /* PCIECLKIC_OE_H_GPIO3_A7 */
  GpioPinSetPull (3, GPIO_PIN_PA7, GPIO_PIN_PULL_NONE);
  GpioPinSetDirection (3, GPIO_PIN_PA7, GPIO_PIN_OUTPUT);
  GpioPinWrite (3, GPIO_PIN_PA7, FALSE);

  GpioPinSetPull (0, GPIO_PIN_PD6, GPIO_PIN_PULL_NONE);
  GpioPinSetDirection (0, GPIO_PIN_PD6, GPIO_PIN_OUTPUT);
  GpioPinWrite (0, GPIO_PIN_PD6, FALSE);
#endif

#if (SATA1_ENABLE  == 1)
  /* SATA1 enable gpio = <&gpio0 RK_PC1 GPIO_ACTIVE_HIGH>; XXX */
  /* gpio = <&gpio3 RK_PA7 GPIO_ACTIVE_HIGH>;              XXX */
  //GpioPinSetDirection (3, GPIO_PIN_PA7, GPIO_PIN_OUTPUT);
  //GpioPinWrite (3, GPIO_PIN_PA7, TRUE);
#endif

}

STATIC
EFI_STATUS
PmicRead (
  IN UINT8 Register,
  OUT UINT8 *Value
  )
{
  return I2cRead (I2C0_BASE, PMIC_I2C_ADDR,
                  &Register, sizeof (Register),
                  Value, sizeof (*Value));
}

STATIC
EFI_STATUS
PmicWrite (
  IN UINT8 Register,
  IN UINT8 Value
  )
{
  return I2cWrite (I2C0_BASE, PMIC_I2C_ADDR,
                  &Register, sizeof (Register),
                  &Value, sizeof (Value));
}

STATIC
VOID
BoardInitPmic (
  VOID
  )
{
  EFI_STATUS Status;
  UINT16 ChipName;
  UINT8 ChipVer;
  UINT8 Value;

  DEBUG ((DEBUG_INFO, "BOARD: PMIC init\n"));

  GpioPinSetPull (0, GPIO_PIN_PB1, GPIO_PIN_PULL_NONE);
  GpioPinSetInput (0, GPIO_PIN_PB1, GPIO_PIN_INPUT_SCHMITT);
  GpioPinSetFunction (0, GPIO_PIN_PB1, 1);
  GpioPinSetPull (0, GPIO_PIN_PB2, GPIO_PIN_PULL_NONE);
  GpioPinSetInput (0, GPIO_PIN_PB2, GPIO_PIN_INPUT_SCHMITT);
  GpioPinSetFunction (0, GPIO_PIN_PB2, 1);

  Status = PmicRead (PMIC_CHIP_NAME, &Value);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to read PMIC chip name! %r\n", Status));
    ASSERT (FALSE);
  }
  ChipName = (UINT16)Value << 4;

  Status = PmicRead (PMIC_CHIP_VER, &Value);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to read PMIC chip version! %r\n", Status));
    ASSERT (FALSE);
  }
  ChipName |= (Value >> 4) & 0xF;
  ChipVer = Value & 0xF;

  DEBUG ((DEBUG_INFO, "PMIC: Detected RK%03X ver 0x%X\n", ChipName, ChipVer));
  ASSERT (ChipName == 0x809);

  /* Initialize PMIC */
  PmicWrite (PMIC_LDO1_ON_VSEL, 0x0c);  /* 0.9V - vdda0v9_image */
  PmicWrite (PMIC_LDO2_ON_VSEL, 0x0c);  /* 0.9V - vdda_0v9 */
  PmicWrite (PMIC_LDO3_ON_VSEL, 0x0c);  /* 0.9V - vdd0v9_pmu */
  PmicWrite (PMIC_LDO4_ON_VSEL, 0x6c);  /* 3.3V - vccio_acodec */
  /* Skip LDO5 for now; 1.8V/3.3V - vccio_sd */
  PmicWrite (PMIC_LDO6_ON_VSEL, 0x6c);  /* 3.3V - vcc3v3_pmu */
  PmicWrite (PMIC_LDO7_ON_VSEL, 0x30);  /* 1.8V - vcca_1v8 */
  PmicWrite (PMIC_LDO8_ON_VSEL, 0x30);  /* 1.8V - vcca1v8_pmu */
  PmicWrite (PMIC_LDO9_ON_VSEL, 0x30);  /* 1.8V - vcca1v8_image */

  PmicWrite (PMIC_POWER_EN1, 0xff); /* LDO1, LDO2, LDO3, LDO4 */
  PmicWrite (PMIC_POWER_EN2, 0xee); /* LDO6, LDO7, LDO8 */
  PmicWrite (PMIC_POWER_EN3, 0x55); /* LDO9, SW1 */
}

EFI_STATUS
EFIAPI
BoardInitDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DEBUG ((DEBUG_INFO, "BOARD: BoardInitDriverEntryPoint() called\n"));

  SocSetDomainVoltage (PMUIO2, VCC_3V3);
  SocSetDomainVoltage (VCCIO1, VCC_3V3);
  SocSetDomainVoltage (VCCIO4, VCC_1V8);
  SocSetDomainVoltage (VCCIO5, VCC_3V3);
  SocSetDomainVoltage (VCCIO6, VCC_1V8);
  SocSetDomainVoltage (VCCIO7, VCC_3V3);

  BoardInitPmic ();

  /* Set GPIO0 PA6 (USER_LED2) output high to enable LED */
  GpioPinSetDirection (0, GPIO_PIN_PA6, GPIO_PIN_OUTPUT);
  GpioPinWrite (0, GPIO_PIN_PA6, TRUE);

  /* Enable automatic clock gating */
  MmioWrite32 (PMU_NOC_AUTO_CON0, 0xFFFFFFFFU);
  MmioWrite32 (PMU_NOC_AUTO_CON1, 0x000F000FU);

  /* Set core_pvtpll ring length */
  MmioWrite32 (GRF_CPU_COREPVTPLL_CON0,
               ((CORE_PVTPLL_RING_LENGTH_SEL_MASK | CORE_PVTPLL_OSC_EN | CORE_PVTPLL_START) << 16) |
               (5U << CORE_PVTPLL_RING_LENGTH_SEL_SHIFT) | CORE_PVTPLL_OSC_EN | CORE_PVTPLL_START);

  /* Configure MULTI-PHY 0 for USB3 mode */
  MultiPhySetMode (0, MULTIPHY_MODE_USB3);
#if (SATA1_ENABLE  == 1)
  /* Configure MULTI-PHY 1 for SATA mode */
  MultiPhySetMode (1, MULTIPHY_MODE_SATA);
#endif
  /* Configure MULTI-PHY 2 for PCIE mode */
  MultiPhySetMode (2, MULTIPHY_MODE_PCIE);

  /* Set GPIO0 PB7 (USB_HOST_PWREN) output high to power USB ports */
  GpioPinSetDirection (0, GPIO_PIN_PB7, GPIO_PIN_OUTPUT);
  GpioPinWrite (0, GPIO_PIN_PB7, TRUE);

   /* PCIe setup */
  DEBUG ((DEBUG_INFO, "BoardInitPci START\n"));
  BoardInitPcie ();
  DEBUG ((DEBUG_INFO, "BoardInitPci END\n"));

  return EFI_SUCCESS;
}
