import asyncio
from datetime import time
from idlelib import rstrip
from signal import pause
from time import sleep

import pygame
from bleak import BleakClient, discover

serviceUuid = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
characteristicUuid = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

pygame.mixer.pre_init(44100, -16, 1, 512)
pygame.mixer.init()
pygame.init()
music=pygame.mixer.Sound("lyd1.ogg")

async def run(loop):
    device = None

    print('looking for devices...')
    while device is None:
        devices = await discover()
        device = next((d for d in devices if d.name == 'Adafruit Bluefruit LE'), None)


    async with BleakClient(device.address, loop=loop) as client:
        queue = asyncio.Queue()
        print('connecting...');
        await client.connect()
        print('starting notificiations');
        def callback(sender, data):
          print(f"{sender}: {data}")
        await client.start_notify(characteristicUuid, lambda _, data: loop.call_soon(queue.put_nowait, data))

        print('waiting')
        while True:
            args = await queue.get()
            b = (args)
            c = b.decode('utf8')[:-2]
            d = c.replace('.', '')
            print (d.rstrip().lstrip())
            if int(d) < int(100):
                print ("Yes!")
                music.play()
                

        # await client.stop_notify(characteristicUuid)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()

    try:
        loop.run_until_complete(run(loop))
    except KeyboardInterrupt:
            loop.close()
