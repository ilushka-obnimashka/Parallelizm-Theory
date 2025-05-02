import logging
import os
import queue
import threading
import time
from abc import ABC
from logging.handlers import RotatingFileHandler
from typing import Tuple

import cv2

if not os.path.exists('log'):
    os.makedirs('log')
try:
    os.remove(os.path.join('log', 'app.log'))
except FileNotFoundError:
    pass

logging.basicConfig(
    level=logging.INFO,
    handlers=[
        RotatingFileHandler('log/app.log', maxBytes=1024 * 1024, backupCount=5)
    ]
)
logger = logging.getLogger(__name__)


class Sensor(ABC):
    def __init__(self, sensor_name):
        self.name = sensor_name
        self._queue = queue.Queue(maxsize=100)
        self._stop_thread_event = threading.Event()

    def start(self):
        self.thread = threading.Thread(target=self._run)
        self.thread.start()

    def stop(self):
        self._stop_thread_event.set()
        self.thread.join()

    def _run(self):
        raise NotImplementedError

    def __del__(self):
        self.stop()

    def get(self):
        try:
            return self._queue.get_nowait()
        except queue.Empty:
            return None

class SensorCam(Sensor):
    def __init__(self, camera_name: str, resolution: Tuple[int, int] = (1280, 720)):
        super().__init__('sensor_cam')
        self.camera_name = camera_name
        self.resolution = resolution
        self.critical_error = threading.Event()

        try:
            self.cap = cv2.VideoCapture(camera_name)
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.resolution[0])
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.resolution[1])

            if not self.cap.isOpened():
                raise IOError("Couldn't open webcam or video")

        except IOError as e:
            logger.error("Couldn't open webcam or video")
            raise e

        except Exception as e:
            logger.error("Camera setup error : ")
            raise e

        finally:
            if hasattr(self, 'cap') and not self.cap.isOpened():
                self.cap.release()

    def _run(self):
        error_count = 0
        while not self._stop_thread_event.is_set():
            is_successfully, frame = self.cap.read()
            if is_successfully:
                try:
                    self._queue.put(frame)
                except Exception as e:
                    logger.error("Error while putting frame: %s", e)
            else:
                error_count += 1
                if error_count >= 10:
                    logger.error("Camera read error: %d consecutive failures", error_count)
                    self.critical_error.set()
                    break
            time.sleep(0.000001)

    def __del__(self):
        self.stop()
        if hasattr(self, 'cap') and self.cap.isOpened():
            self.cap.release()


def SensorXAdapter(name: str):
    def class_decorator(cls):
        class Adapter(Sensor):
            def __init__(self, delay):
                super().__init__(f"{name}{str(delay)}")
                self._sensor_x = cls(delay)

            def _run(self):
                while not self._stop_thread_event.is_set():
                    value = self._sensor_x.get()
                    try:
                        self._queue.put(value)
                    except Exception as e:
                        logger.error("Error while putting value: %s", e)
        return Adapter
    return class_decorator

@SensorXAdapter(name="SensorX")
class SensorX:
    def __init__(self, delay):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data = self._data + 1
        return self._data



@SensorXAdapter(name="SensorX")
class SensorX():
    def __init__(self, delay):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data = self._data + 1
        return self._data


class WindowImage:
    def __init__(self, display_freq):
        self.display_freq = display_freq
        self.window_name = "Sensor Display"
        self._last_update = 0
        cv2.namedWindow(self.window_name, cv2.WINDOW_NORMAL)
        self.last_values = {}

    def show(self, camera_frame, sensor_values):
        current_time = time.time()

        if current_time - self._last_update < 1.0 / self.display_freq:
            return

        self._last_update = current_time

        if camera_frame is not None:
            display_frame = camera_frame.copy()
            y = 30
            for name, value in sensor_values.items():
                text = f"{name}: {value}"
                cv2.putText(display_frame, text, (10, y),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                y += 30
            cv2.imshow(self.window_name, display_frame)
            cv2.waitKey(1)

    def __del__(self):
        try:
            cv2.destroyWindow(self.window_name)
        except cv2.error:
            pass
