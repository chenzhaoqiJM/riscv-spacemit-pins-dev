# GPIO调试

## 依赖安装

```
sudo apt install libgpiod-dev liblgpio-dev python3-gpiozero
```


## 查看gpio信息

通 过 debugfs 文 件 系 统 查 看 /sys/kernel/debug/目录下的相关文件，可以获取 GPIO 的状态

### 查看正在使用的gpio信息

```
cat /sys/kernel/debug/gpio 
```

注意，这只显示正在使用中的gpio的状态



### /sys/kernel/debug/pinctrl/*/pinmux-pins 目录

- pinmux-pins : 列出了每个 GPIO 引脚的引脚复用配置。

- pins ：查看 GPIO 对应的引脚编号

- gpio-ranges：列 出 了 每 个 GPIO 控 制 器 支 持 的 GPIO 范 围。

- pinmux-functions：下 列 出 了 每 个 功 能 模 式 的 名 称 以 及 与 对 应 的 GPIO 引 脚。

- pingroups：下提供有关用于配置和控制系统上的 GPIO 引脚的引脚组的信息。

- pinconf-pins：GPIO 引脚的配置信息，如输入/输出模式、上拉/下拉设置等

一般，使用目录 `/sys/kernel/debug/pinctrl/d401e000.pinctrl-pinctrl-single` 即可

进入 `/sys/kernel/debug/pinctrl/d401e000.pinctrl-pinctrl-single` 目录

较有用的是：pinmux-pins 文件，通过该文件查看复用功能的设置

在使用输入输出功能时，尽量选择未配置的引脚。`cat pinmux-pins` 时显示为

```
pin 72 (PIN72): UNCLAIMED
```



## sys 操作 gpio

```
cd /sys/class/gpio/
```

export：用于将指定编号的 GPIO 引脚导出

```
echo 49 > export 
```

会生成 gpio49 文件夹

```
echo 49 > unexport
```

**gpiox 文件夹：**

direction：配置 GPIO 引脚为输入或输出模式。该文件可读、可写， echo in 或者 out

active_low：用于控制极性的属性文件，可读可写，默认情况下为 0，当 active_low 等于 0 时，value 值为 1 则引脚输出高电平，value 值为 0 则引脚输出低电平

value：设置引脚高低电平, 0 或 1

edge：控制中断的触发模式，该文件可读可写，可配置：none、rising、falling、both