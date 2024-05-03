/** @file
*  Differentiated System Description Table Fields (DSDT) for the Radxa E25.
*
*  Copyright (c) 2023, Jared McNeill <jmcneill@invisible.ca>
*  Copyright (c) 2023, Sergey Tyuryukanov <s199p.wa1k9r@gmail.com>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <IndustryStandard/Acpi60.h>

DefinitionBlock ("DsdtTable.aml", "DSDT",
                 EFI_ACPI_6_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_REVISION,
                 "RKCP  ", "RK356X  ", FixedPcdGet32 (PcdAcpiDefaultOemRevision)) {
  Scope (_SB) {
    include ("Cpu.asl")
    include ("Tsadc.asl")
    include ("Uart.asl")
    include ("Wdt.asl")
    include ("Usb2.asl")
    include ("Usb3.asl")
    include ("Mshc.asl")
    include ("Emmc.asl")
    include ("Sata.asl")
    include ("Pcie2x1.asl")
    include ("Pcie3x1.asl")
    include ("Pcie3x2.asl")
  } // Scope (_SB)
}
