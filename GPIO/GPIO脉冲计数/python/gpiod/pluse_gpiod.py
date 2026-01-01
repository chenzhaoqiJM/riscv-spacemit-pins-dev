import gpiod
from gpiod.line import Edge, Direction, Bias
import time
import threading
import queue
import signal
import sys

class EncoderSpeedMeter:
    def __init__(
        self,
        gpio_offset: int,
        chip_path: str = "/dev/gpiochip0",
        sample_period: float = 0.033,
        encoder_ppr: float = 11.0,
        gear_ratio: float = 56.0,
        queue_size: int = 10,
    ):
        # 参数
        self.chip_path = chip_path
        self.gpio_offset = gpio_offset
        self.sample_period = sample_period
        self.encoder_ppr = encoder_ppr
        self.gear_ratio = gear_ratio

        # 状态
        self._count = 0
        self._speed_filt = 0.0
        self._has_rising = False  # 状态位：标记是否捕获到了上升沿

        # 同步与通信
        self._lock = threading.Lock()
        self._stop_event = threading.Event()
        self._queue = queue.Queue(maxsize=queue_size)

        # 线程初始化
        self._interrupt_thread = threading.Thread(target=self._listen_interrupts, daemon=True)
        self._sampler = threading.Thread(target=self._sampler_loop, daemon=True)
        self._processor = threading.Thread(target=self._processor_loop, daemon=True)

    # ---------------- 中断监听线程 (gpiod 核心) ----------------
    def _listen_interrupts(self):
        """使用 gpiod 监听双边沿事件"""
        with gpiod.Chip(self.chip_path) as chip:
            line_config = {
                self.gpio_offset: gpiod.LineSettings(
                    direction=Direction.INPUT,
                    edge_detection=Edge.BOTH,  # 必须监听双边沿来实现逻辑
                    bias=Bias.PULL_UP          # 根据硬件情况可选 PULL_UP/PULL_DOWN
                )
            }

            with chip.request_lines(config=line_config) as request:
                while not self._stop_event.is_set():
                    # 等待事件，超时设为 0.1s 以便能响应 stop_event
                    if request.wait_edge_events(timeout=0.1):
                        events = request.read_edge_events()
                        for event in events:
                            self._handle_event(event)

    def _handle_event(self, event):
        """逻辑：上升沿锁定，随后的下降沿计数"""
        with self._lock:
            # event.event_type 在 gpiod 2.x 中通过枚举判断
            if event.event_type == event.Type.RISING_EDGE:
                self._has_rising = True
            elif event.event_type == event.Type.FALLING_EDGE:
                if self._has_rising:
                    self._count += 1
                    self._has_rising = False  # 重置状态，等待下一个脉冲

    # ---------------- 采样线程 ----------------
    def _sampler_loop(self):
        last_time = time.monotonic()
        while not self._stop_event.is_set():
            time.sleep(self.sample_period)

            now = time.monotonic()
            dt = now - last_time
            last_time = now

            with self._lock:
                pulse = self._count
                self._count = 0
            
            if dt > 0:
                try:
                    self._queue.put_nowait((dt, pulse))
                except queue.Full:
                    pass

    # ---------------- 计算线程 ----------------
    def _processor_loop(self):
        while not self._stop_event.is_set():
            try:
                dt, pulse = self._queue.get(timeout=0.2)
            except queue.Empty:
                continue

            # 计算转速 (基于逻辑：1个脉冲代表1个完整的PPR周期)
            # 因为我们逻辑里是 上升+下降 = count+1，所以这里不需要再除以 2
            encoder_circles = pulse / self.encoder_ppr
            motor_circles = encoder_circles / self.gear_ratio
            self._speed_filt = motor_circles / dt

            print(f"脉冲数: {pulse} | dt: {dt*1000:.1f}ms | 转速: {self._speed_filt*60:.2f} RPM")

    # ---------------- 接口 ----------------
    def start(self):
        self._interrupt_thread.start()
        self._sampler.start()
        self._processor.start()

    def stop(self):
        self._stop_event.set()
        self._interrupt_thread.join()
        self._sampler.join()
        self._processor.join()

def main():
    meter = EncoderSpeedMeter(
        gpio_offset=71,
        chip_path="/dev/gpiochip0",
        sample_period=0.033,
        encoder_ppr=11.0,
        gear_ratio=30.0
    )

    signal.signal(signal.SIGINT, lambda s, f: sys.exit(0))
    
    print("开始测量...")
    meter.start()
    
    while True:
        time.sleep(1)

if __name__ == "__main__":
    main()