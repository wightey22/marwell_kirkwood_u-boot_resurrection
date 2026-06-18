# marwell_kirkwood_u-boot_resurrection
Try to port old Marwell's u-boot patches (for Buffalo LX-WXL and Seagate NAS440) onto more actual u-boot sources 

---

## Licensing and Provenance

This repository contains historical source code releases and modifications for specific NAS and embedded platforms based on the **Marvell Kirkwood (88F6281)** SoC architecture. Because this repository aggregates code from multiple legacy vendors and upstream open-source projects, it is distributed under a **multi-license structure**.

Please refer to the specific license files in the root directory for full legal terms.

### 1. Seagate BlackArmor NAS 440 (`/src/Seagate/BA_NAS_4xx_GPL`)
* **Core Components:** Linux Kernel, user-space tools, and system modifications provided by Seagate.
* **Primary License:** **GNU General Public License v2 (GPL-2.0)**.
* **License File:** See `/src/Seagate/BA_NAS_4xx_GPL/BA_NAS_License.txt` and other files in `/src/Seagate/BA_NAS_4xx_GPL/` directory (or the original `COPYING` file inside the source tree).
* **Details:** Under the terms of the GPLv2, all modifications made by Seagate to the Linux kernel and other GPL-licensed utilities are openly available. Any proprietary user-space management applications or diagnostics tools included by the vendor retain their respective copyrights.

### 2. Buffalo LinkStation Duo LS-WXL (`/src/buffalo/ls-wxl`)
* **Core Components:** Bootloader source code and patch files for "Das U-Boot" version 1.1.4 tailored for Marvell Kirkwood boards.
* **Primary License:** **GNU General Public License v2 (GPL-2.0) or later**.
* **License File:** See the original `COPYING` file inside the source tree.
* **Details:** "Das U-Boot" is strictly licensed under the GPL. Buffalo's hardware-specific patches, board initializations, and machine definitions for the LS-WXL series are distributed under the same terms.

---

*Disclaimer: This repository is maintained for historical preservation, reverse engineering, and homebrew firmware development. It is not affiliated with, endorsed by, or supported by Seagate Technology PLC, Buffalo Americas Inc., or Marvell Technology Group.*

