import asyncio
import websockets
import cv2
import mediapipe as mp
from mediapipe.tasks import python
from mediapipe.tasks.python import vision

# Victory
# Pointing_Up
# Thumb_Up
# Open_Palm
# ILoveYou
# Thumb_Down


### BUG:  大量消息发送堆积导致错误

base_options = python.BaseOptions(model_asset_path='gesture_recognizer.task')
options = vision.GestureRecognizerOptions(base_options=base_options)
recognizer = vision.GestureRecognizer.create_from_options(options)
cap = cv2.VideoCapture(0)  # 打开摄像头
mp_hands = mp.solutions.hands

async def calculate(websocket):
    with mp_hands.Hands(min_detection_confidence=0.5, min_tracking_confidence=0.5) as hands:
        while cap.isOpened():
            success, image = cap.read()
            if not success:
                print("无法读取视频流")
                break

            # 将图像转换为RGB格式
            image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

            # 检测手部
            results = hands.process(image_rgb)
            # 一定要转换成mp支持的格式
            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=image_rgb)

            # 推理
            recognition_result = recognizer.recognize(mp_image)
            if recognition_result.gestures:
                # 打印推理结果
                print(recognition_result.gestures[0][0].category_name)
                await websocket.send(recognition_result.gestures[0][0].category_name)
                await asyncio.sleep(2)


async def send_message(websocket, result):
    # 发送消息
    await websocket.send(str(result))


async def receive_message(websocket):
    while True:
        mesg = await websocket.recv()
        print(mesg)


async def main():
    async with websockets.connect("ws://localhost:8765") as websocket:
        # asyncio.create_task(receive_message(websocket))  # 这里不用等待，不管它，如果等待就会卡在这个任务里
        while True:
            # 创建协程计算结果
            task_calculate = asyncio.create_task(calculate(websocket))
            # 等待计算完成
            result = await task_calculate
            # 创建协程发送消息
            task_send_message = asyncio.create_task(send_message(websocket, result))
            # 等待发送完成
            await asyncio.gather(task_send_message)


if __name__ == "__main__":
    asyncio.run(main())
