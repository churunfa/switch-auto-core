该项目负责与ESP32虚拟手柄交互，提供最基础最核心的功能。该项目通过gRPC将基础功能暴露出去，供web端操作。

同时也支持接收其他手柄的信号，与按键宏协调工作。


需要跟以下项目搭配部署
ESP32固件：https://github.com/churunfa/SwitchProControllerEsp32S3

web端服务代码：https://github.com/churunfa/switch-auto-web


其他项目：

前端代码：https://github.com/churunfa/switch-auto-app  （当前已集成到web端代码中）

小程序（当前在备案中，预计3月份上架，可以提前申请体验名额）：

![IMG_4161](https://github.com/user-attachments/assets/11078e27-209d-46ba-b93d-905ff5c5d91e)


pc端操作页面：

1、基础按键模拟：该页面包含所有基础按键，可以进行简单的触发和释放操作。ps:左遥感旋转一圈为废案，当前不可用，后期会删除。

<img width="1725" height="945" alt="image" src="https://github.com/user-attachments/assets/7cc6c6c6-265d-445f-8175-bcd2d0b32a1c" />

2、拓扑图配置页面：该页面用于配置按键宏，可以毫秒级精准进行按键编排

<img width="3448" height="1894" alt="image" src="https://github.com/user-attachments/assets/c44a8534-5a57-4155-9238-aea9d613170f" />、

3、按键绑定：当连接手柄时，可以在这里修改按键映射。也可以进行功能键设置和拓扑图宏绑定，当前宏按键触发逻辑为 按住功能键再按拓扑图宏按键触发

<img width="1722" height="953" alt="image" src="https://github.com/user-attachments/assets/f88bd620-0543-4f7d-9fa2-000e1838719f" />

4、斯普拉遁涂鸦绘制：可以将图片自动转化为单色位图，并全自动进行涂鸦绘制

<img width="3438" height="1900" alt="image" src="https://github.com/user-attachments/assets/f465a184-8ff8-485d-83e0-c5553078ac4c" />

小程序：
1、首页：唤醒NS(手机远程唤醒NS-当前只支持NS2)、重启(重启虚拟手柄)、连接手柄(Switch主机要求输入L+R再按A切换手柄时，可以通过这里操作)、脚本循环（支持将特定拓扑图宏写入虚拟手柄，控制宏循环执行）、脚本停止（停止宏循环）

<img width="603" height="1311" alt="IMG_4163" src="https://github.com/user-attachments/assets/c1331a6c-3df2-4f31-8da0-f8b12bc1808a" />

2、拓扑图上传：支持上传拓扑图，可以将其他人导出的拓扑宏配置粘贴到这里进行上传

<img width="603" height="1311" alt="IMG_4164" src="https://github.com/user-attachments/assets/fdb1b80d-8914-488b-8c71-eecbbf7fed49" />

3、固件更新：支持更新虚拟手柄的固件版本

<img width="603" height="1311" alt="IMG_4166" src="https://github.com/user-attachments/assets/73d456d2-afea-4ebd-a725-00005f2f7c1e" />

4、NS2唤醒配置：第一次唤醒需要写入真实设备的信息，支持将SwitchPro/JC手柄贴近虚拟手柄，自动失败设备信息。同时也支持手动写入。

<img width="603" height="1311" alt="IMG_4167" src="https://github.com/user-attachments/assets/b0179d5a-1a1e-4752-93c2-5940b5d8f831" />
