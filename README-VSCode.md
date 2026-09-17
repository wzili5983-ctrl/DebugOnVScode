# VS Code + OpenOCD 在线调试

本工程为 STM32F407ZG，使用 ST-LINK 的 SWD 接口。保留 Keil ARM Compiler 5 编译，OpenOCD 负责连接和下载，ARM GDB + Cortex-Debug 负责源码调试。OpenOCD 本身不是 C 编译器。

## 使用

1. 在 VS Code 中打开本工程根目录，安装推荐的 Cortex-Debug 扩展（本机已安装）。
2. ST-LINK 连接目标板的 SWDIO（PA13）、SWCLK（PA14）、GND，并按探针要求连接目标电压参考；目标板正常供电。关闭 Keil、STM32CubeIDE 等占用探针的调试会话。
3. 在“运行和调试”中选择 `STM32F407 - Keil Build + OpenOCD`，按 F5。编译成功后自动下载并停在 `main`。
4. F9 设置断点，F10 单步跳过，F11 单步进入，F5 继续；左侧可查看变量、监视表达式和调用栈。每次按 F5 会先由 `.vscode/tasks.json` 中的 `Keil: Build LED` 任务调用 `UV4.exe -r` 命令行重新编译。

Ctrl+Shift+B 可单独编译。第二个调试配置直接使用现有 `OBJ/LED.axf` 和 `OBJ/LED.hex`，需要确保它们来自同一次编译且与源码一致。

按 `Ctrl+F5`（Run Without Debugging）现在会选择 `STM32F407 - Build + Download (Ctrl+F5)`：重新编译工程，下载 HEX 并复位设备，不在源码中启动调试。

### 编译后下载，不进入 DEBUG

在菜单“终端 → 运行任务”中选择 `Keil: Build + Download LED (no debug)`。任务会依次重新编译、下载 `OBJ/LED.hex`、校验并复位运行程序，完成后退出 OpenOCD，不启动调试会话，也不会停在 `main`。编译失败时不会执行下载，编译警告允许继续。

若已经编译，只需下载，选择 `OpenOCD: Download LED (no debug)`；此任务直接使用现有 HEX，不会重新编译。下载前先停止正在运行的调试会话，释放 ST-LINK。

## 本机路径

- Keil：`D:/Keil 5/UV4/UV4.exe`，在 `scripts/build-keil.ps1` 的 `KeilPath` 默认值中修改。
- OpenOCD：`C:/msys64/ucrt64/bin/openocd.exe`。
- ARM GDB：复用本机 STM32CubeIDE 2.0.0 中的 GNU 工具，在 `.vscode/settings.json` 中修改工具目录。
- 探针及芯片配置：`debug/openocd.cfg`，默认 ST-LINK、SWD、1000 kHz。

调试符号读取 AXF，烧录使用同次编译的 HEX，避免 Keil AXF 的 RAM 段布局影响 GDB 下载。`cwd` 指向 USER，使 Keil 的相对源码路径能够解析。

## 排查

- 找不到 ST-LINK：检查 USB、供电及 ST-LINK 驱动，并关闭其他调试器。
- 无法连接芯片：检查 SWD 接线，可将 `adapter speed` 降到 100。若程序禁用了 SWD，连接 NRST 后将 `reset_config none` 改为 `reset_config srst_only srst_nogate connect_assert_srst`。
- 源码单步跳行或变量被优化：在 Keil 的 C/C++ 设置中将 Optimization 设为 Level 0，并保留 Debug Information，重新编译。
- 编译失败：查看终端和 `OBJ/vscode-build.log`；本工程要求 Keil ARM Compiler 5.06 update 7。
- 更换为 CMSIS-DAP：把接口改为 `interface/cmsis-dap.cfg`，传输改为 `swd`。

配置文件与离线检查不能替代真实硬件验证；需要连接开发板后确认下载、断点及单步运行。
