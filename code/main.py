import serial
import threading
import time
import numpy as np
import sounddevice as sd
import soundfile as sf

# ==========================================================
# CONFIG
# ==========================================================

PORT = "COM7"
BAUD = 115200

BASELINE = 550000
PEAK_THRESHOLD = 10000
LEFT_HAND_THRESHOLD = 700000

STANDARD_FORCE = 590000

LOOP_START_MS = 180

AUDIO_PATH = r"C:\Users\wya48\Desktop\Guzheng project\NoodlePadoodle\audio\M_E\MdE_m.wav"

# ==========================================================
# LOAD AUDIO
# ==========================================================

audio, sample_rate = sf.read(AUDIO_PATH)

if len(audio.shape) > 1:
    audio = audio.mean(axis=1)

loop_start = int(sample_rate * LOOP_START_MS / 1000)

# ==========================================================
# SERIAL
# ==========================================================

serial_port = serial.Serial(PORT, BAUD)

def read_force():
    while True:
        try:
            line = serial_port.readline().decode().strip()
            if line:
                return int(line)
        except:
            pass

# ==========================================================
# GLOBALS
# ==========================================================

playing = False
stop_flag = False
first_play = True

# ==========================================================
# AUDIO
# ==========================================================

def play_audio(volume):

    global playing
    global stop_flag
    global first_play

    playing = True
    stop_flag = False

    pointer = 0 if first_play else loop_start
    first_play = False

    while not stop_flag:

        if pointer >= len(audio):
            pointer = loop_start

        chunk = audio[pointer:pointer+2048] * volume

        sd.play(chunk, sample_rate, blocking=True)

        pointer += len(chunk)

    playing = False

# ==========================================================
# START SOUND
# ==========================================================

def trigger_string(peak):

    global stop_flag

    energy = peak - BASELINE
    standard_energy = STANDARD_FORCE - BASELINE

    volume = max(0.05, energy / standard_energy)

    print("\n==============================")
    print("PLUCK!")
    print("Peak =", peak)
    print("Volume =", round(volume,3))

    stop_flag = True

    while playing:
        time.sleep(0.005)

    threading.Thread(
        target=play_audio,
        args=(volume,),
        daemon=True
    ).start()

# ==========================================================
# LEFT HAND MODE
# ==========================================================

def left_hand_mode():

    print("\n******** LEFT HAND ********")

    while True:

        force = read_force()

        pitch = 1 + (force - LEFT_HAND_THRESHOLD) / (STANDARD_FORCE - BASELINE)

        print(f"Force={force}   Pitch={pitch:.3f}")

        # TODO
        # 下一版这里真正修改声音Pitch

        if force < LEFT_HAND_THRESHOLD:
            print("Left hand released.\n")
            return

# ==========================================================
# PEAK DETECTOR
# ==========================================================

def detect_peak(first):

    a = first
    b = read_force()

    while True:

        if b > LEFT_HAND_THRESHOLD:

            left_hand_mode()

            return None

        c = read_force()

        if a < b and b > c:
            return b

        a = b
        b = c

# ==========================================================
# MAIN
# ==========================================================

print("=================================")
print(" NoodlePadoodle Demo V1")
print(" Ready!")
print("=================================")

while True:

    point = read_force()

    if point - BASELINE > PEAK_THRESHOLD:

        peak = detect_peak(point)

        if peak is not None:

            trigger_string(peak)