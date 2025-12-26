# PWM调试

请先看 **设备树配置说明**

## 导出

echo 0 > pwmchip0/export

这里的 0 表示：`pwmchip0` 这个 PWM 控制器里的第 0 路 PWM 通道（channel index）

## 设置PWM频率

一个周期的时间，单位为ns，即一个周期为1KHZ

echo 1000000 > pwmchip0/pwm0/period 

## 使能PWM

echo 1 > pwmchip0/pwm0/enable 

## 设置PWM占空比
echo 500000 > pwmchip0/pwm0/duty_cycle 

调节占空比，可以反复执行

echo 50000 > pwmchip0/pwm0/duty_cycle 

## 关闭PWM

echo 0 > pwmchip0/pwm0/enable 

## 取消导出

echo 0 > pwmchip0/unexport