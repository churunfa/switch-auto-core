# Switch Auto Core

`Switch Auto Core` 是整个 Switch 自动化控制系统的底层核心组件。它负责与基于 ESP32 的虚拟手柄硬件进行底层交互，并通过 gRPC 协议对外暴露核心控制接口，是连接硬件与前端应用的桥梁。

---

## 🚀 项目定位

该项目专注于提供最基础、最核心的控制能力：
- **底层交互**：实现与 ESP32 虚拟手柄的高效通信。
- **接口暴露**：通过 **gRPC** 将手柄控制逻辑抽象化，供 Web 端调用。
- **信号协调**：支持接收物理手柄信号，并与自定义的按键宏逻辑协同工作。

---

## 🏗 生态系统（配套组件）

为了实现完整功能，本项目建议与以下组件配套使用：

* **ESP32 固件**: [SwitchProControllerEsp32S3](https://github.com/churunfa/SwitchProControllerEsp32S3) (硬件核心)
* **Web 服务端**: [switch-auto-web](https://github.com/churunfa/switch-auto-web) (业务逻辑层)
* **前端代码**: [switch-auto-app](https://github.com/churunfa/switch-auto-app) (目前已集成至 Web 端代码中)

---

## ✨ 核心功能展示

### 1. PC 端操作页面

| 功能模块 | 说明 | 界面预览 |
| :--- | :--- | :--- |
| **基础按键模拟** | 提供全键位模拟操作，支持触发与释放。*(注：左摇杆旋转一周为废弃功能，后续将移除)* | <img src="https://github.com/user-attachments/assets/7cc6c6c6-265d-445f-8175-bcd2d0b32a1c" width="450" /> |
| **拓扑图配置** | 毫秒级精准按键编排，支持可视化逻辑连线，轻松创建复杂宏脚本。 | <img src="https://github.com/user-attachments/assets/c44a8534-5a57-4155-9238-aea9d613170f" width="450" /> |
| **按键绑定映射** | 自由修改物理手柄映射，支持“功能键 + 拓扑宏”的组合触发逻辑。 | <img src="https://github.com/user-attachments/assets/f88bd620-0543-4f7d-9fa2-000e1838719f" width="450" /> |
| **斯普拉遁涂鸦** | 自动将图片转换为单色位图，全自动控制手柄在游戏中进行涂鸦绘制。 | <img src="https://github.com/user-attachments/assets/f465a184-8ff8-485d-83e0-c5553078ac4c" width="450" /> |

---

### 2. 小程序端 (移动端便捷控制)

> <img src="https://github.com/user-attachments/assets/11078e27-209d-46ba-b93d-905ff5c5d91e" width="165" />

#### 移动端亮点：
* **远程唤醒**：支持远程唤醒 NS (目前已适配 NS2)。
* **脚本循环**：支持将特定拓扑宏写入硬件，**脱离 PC** 实现自动化循环执行。
* **固件更新**：通过手机端 OTA 直接为虚拟手柄更新最新版固件。
* **NS2 唤醒配置**：支持读取真实手柄信息（贴近感应或手动输入）以完成唤醒授权。

<p align="center">
  <img src="https://github.com/user-attachments/assets/06b7a8dc-565d-42f1-9a59-9a4f0c54f19d" width="180" alt="首页" />
  <img src="https://github.com/user-attachments/assets/fdb1b80d-8914-488b-8c71-eecbbf7fed49" width="180" alt="上传" />
  <img src="https://github.com/user-attachments/assets/73d456d2-afea-4ebd-a725-00005f2f7c1e" width="180" alt="更新" />
  <img src="https://github.com/user-attachments/assets/b0179d5a-1a1e-4752-93c2-5940b5d8f831" width="180" alt="唤醒" />
</p>

---

## 🛒 硬件及全家桶套餐

以上功能均可基于本项目自行部署使用。如果您希望获得“开箱即用”的体验，可以关注我们的 **ESP32 开发版成品套餐**（https://e.tb.cn/h.iYG1toOrUpXmIRa?tk=tb9sUI5K565 ）。

**套餐包含：**
1. ✅ **硬件**：预装专属固件的 ESP32 开发版。
2. ✅ **移动端**：微信小程序永久使用权限。
3. ✅ **客户端**：集成了上述所有功能的专属控制台软件（支持 Windows / MacOS）。

<p align="center">
  <img src="https://github.com/user-attachments/assets/ea811ea9-988e-4fb8-b3a6-c5c652060dd0" width="600" alt="套餐示意图" />
</p>

视频介绍：https://www.bilibili.com/video/BV1SVPTzhEs2/?spm_id_from=333.1387.list.card_archive.click
