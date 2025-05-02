import click

from all_classes import *


@click.command()
@click.option("--camera_name", "-c", default="/dev/video0", type=str, help="Name of the camera.")
@click.option("--resolution", "-r", default=(1280, 720), type=(int, int), help="Resolution of the camera.")
@click.option("--fps", "-f", default=144, type=int, help="The frequency of display of the picture.")
def main(camera_name, resolution, fps):
    try:
        camera = SensorCam(camera_name, resolution)
        sensors = [
            SensorX(0.01),
            SensorX(0.1),
            SensorX(1.0)
        ]

        camera.start()
        for sensor in sensors:
            sensor.start()

        window = WindowImage(fps)
        last_values = {}
        last_frames = []
        max_frames = 2

        while True:
            if camera.critical_error.is_set():
                logger.critical("Camera critical error detected. Exiting...")
                break

            frame = camera.get()
            if frame is not None:
                last_frames.append(frame)
                if len(last_frames) > max_frames:
                    last_frames.pop(0)
            current_frame = last_frames[-1] if last_frames else None

            sensor_values = {}
            for sensor in sensors:
                value = sensor.get()
                sensor_values[sensor.name] = last_values.get(sensor.name, 0) if value is None else value
                last_values[sensor.name] = sensor_values[sensor.name]
            window.show(current_frame, sensor_values)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except Exception as e:
        logger.critical(f"Fatal error: {str(e)}", exc_info=True)
    finally:
        if hasattr(camera, 'thread'):
            camera.stop()
        for sensor in sensors:
            sensor.stop()


if __name__ == '__main__':
    main()
