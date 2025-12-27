# GPIO 设备树配置



## 引脚定义

`linux-6.6/include/dt-bindings/pinctrl/k1-x-pinctrl.h`

该文件定义了引脚编号，以及其它的一些宏



## dts配置

### GPIO 描述

声明：本方案里用到了哪些 GPIO，

以及这些 GPIO 分别接到了哪几个“物理引脚(PAD)”上。

Linux GPIO 框架必须先知道这张 **“GPIO 与 PAD 对照表”**， 否则无法在 Linux 里操作 GPIO。

```
&gpio{
	gpio-ranges = <
		&pinctrl 49  GPIO_49  2
		&pinctrl 58  GPIO_58  1
		&pinctrl 63  GPIO_63  1
		&pinctrl 65  GPIO_65  1
		&pinctrl 67  GPIO_67  1
		&pinctrl 70  PRI_TDI  4
		&pinctrl 74  GPIO_74  1
		&pinctrl 79  GPIO_79  1
		&pinctrl 80  GPIO_80  4
		&pinctrl 90  GPIO_90  3
		&pinctrl 96  DVL0     2
		&pinctrl 110 GPIO_110 1
		&pinctrl 111 GPIO_111 1
		&pinctrl 113 GPIO_113 1
		&pinctrl 114 GPIO_114 3
		&pinctrl 118 GPIO_118 1
		&pinctrl 123 GPIO_123 5
	>;
};
```

**例如：&pinctrl 70 PRI_TDI 4 表示：**

GPIO 控制器里的 GPIO70 ~ GPIO73，
对应 pinctrl 里的 PRI_TDI 开始的 4 个物理引脚。

PRI_TDI  在 `linux-6.6/include/dt-bindings/pinctrl/k1-x-pinctrl.h`  里面定义



### GPIO pin 配置

**把上面登记过的 GPIO 对应的物理引脚：**

- 切成 GPIO 模式 / 其他功能
- 配上拉 / 下拉
- 配驱动强度
- 配中断边沿

```
&pinctrl {
	pinctrl-single,gpio-range = <
		&range GPIO_49  2 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_3V_DS4)
		&range GPIO_58  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_63  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_64  1 (MUX_MODE1 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_65  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_67  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_3V_DS4)
		&range PRI_TDI  2 (MUX_MODE1 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range PRI_TCK  1 (MUX_MODE1 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range PRI_TDO  1 (MUX_MODE1 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_74  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_79  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_80  1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_3V_DS4)
		&range GPIO_81  3 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_90  1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_91  2 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range DVL0     1 (MUX_MODE1 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range DVL1     1 (MUX_MODE1 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS0)
		&range GPIO_110 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_111 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_113 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_114 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_115 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
		&range GPIO_116 1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_118 1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_123 1 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS0)
		&range GPIO_124 1 (MUX_MODE0 | EDGE_NONE | PULL_UP   | PAD_1V8_DS2)
		&range GPIO_125 3 (MUX_MODE0 | EDGE_NONE | PULL_DOWN | PAD_1V8_DS2)
	>;
```

**例如：&range GPIO_49 2 (MUX_MODE0 | EDGE_NONE | PULL_UP | PAD_3V_DS4)**

从 **GPIO_49 开始的 2 个物理引脚**：

- 复用模式：**GPIO 模式（MUX_MODE0）**
- 不做中断
- 内部上拉
- 3.3V IO，驱动强度等级 4