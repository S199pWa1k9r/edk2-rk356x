#
# Copyright (c) 2021, Jared McNeill <jmcneill@invisible.ca>
# Copyright (c) 2023, Sergey Tyuryukanov <s199p.wa1k9r@gmail.com>
#
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

#
# Board specific initialization
#
INF Platform/Radxa/E25/Drivers/BoardInitDxe/BoardInitDxe.inf

#
# Devicetree support
#
INF Platform/Rockchip/Rk356x/Drivers/FdtDxe/FdtDxe.inf

#
# Realtek Network support
#
!include NetworkPkg/Network.fdf.inc
!if $(RTL_ENABLE) == TRUE
#
# Realtek PCIe UNDI Driver
#
INF Drivers/Realtek/Bus/Pcie/PcieNetworking/RtkUndiDxe.inf
!endif

!ifdef $(INCLUDE_TFTP_COMMAND)
  INF ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
!endif #$(INCLUDE_TFTP_COMMAND)

#
# ACPI Support
#
INF MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
INF MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
INF Platform/Rockchip/Rk356x/Drivers/PlatformAcpiDxe/PlatformAcpiDxe.inf
INF RuleOverride = ACPITABLE Platform/Rockchip/Rk356x/AcpiTables/$(PLATFORM_NAME).inf

#
# SMBIOS Support
#
INF Platform/Rockchip/Rk356x/Drivers/PlatformSmbiosDxe/PlatformSmbiosDxe.inf
INF MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf

