"""Quick script to watch the BLE‑UART COM port and plot ECG/impedance.
Requires: pip install pyserial matplotlib"""
import sys, serial, collections, matplotlib.pyplot as plt, matplotlib.animation as anim

PORT   = sys.argv[1] if len(sys.argv) > 1 else "/dev/tty.usbserial-XXXX"
BAUD   = 9600
N_SAMP = 500

ecgs = collections.deque(maxlen=N_SAMP)
imps = collections.deque(maxlen=N_SAMP)

ser = serial.Serial(PORT, BAUD)
fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
ln1, = ax1.plot([], [], lw=1)
ln2, = ax2.plot([], [], lw=1)
ax1.set_ylabel("ECG (mV)")
ax2.set_ylabel("Resp µV")
ax2.set_xlabel("sample")


def update(frame):
    line = ser.readline().decode().strip()
    try:
        ecg, imp = map(float, line.split(','))
        ecgs.append(ecg)
        imps.append(imp)
        ln1.set_data(range(len(ecgs)), list(ecgs))
        ln2.set_data(range(len(imps)), list(imps))
        ax1.relim(); ax1.autoscale_view()
        ax2.relim(); ax2.autoscale_view()
    except ValueError:
        pass
    return ln1, ln2

ani = anim.FuncAnimation(fig, update, interval=20)
plt.show()

# python sanity_check.py COM5  # Windows
# python sanity_check.py /dev/tty.usbmodem14101  # macOS/Linux
