import numpy as np
import struct


# CRC16 check
def crc16(data):
    crc = 0xFFFF
    poly = 0x8005

    for byte in data:
        crc ^= byte << 8

        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ poly) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc


# Remove PN9 scrambling
def pn9_descramble(data):
    st = 0x1FF
    out = bytearray()

    for byte in data:
        mask = 0

        for j in range(8):
            mask |= (st & 1) << j

            xor_state = (st & 1) ^ ((st >> 5) & 1)

            st >>= 1

            if xor_state:
                st |= 0x100

        out.append(byte ^ mask)

    return bytes(out)


# Parse image packet fields
def parse_geoscan_telemetry_packet(payload: bytes):
    print(payload.hex())
    if len(payload) < 82:
        print(f"Ошибка: Короткий пакет {len(payload)}")
        return None

    if payload[32] != 0x01:
        print(f"Неверный Mayak ID: {hex(payload[32])} (ожидалось 0x01)")
        return None

    print("Геоскан: Пакет Телеметрии")

    # EPS Block
    time_unix = struct.unpack_from("<I", payload, 25)[0]  # Offset 17+8
    eps_mode = payload[29]  # Offset 21+8
    current_load_ma = struct.unpack_from("<H", payload, 31)[0]  # Offset 23+8
    current_solar_ma = struct.unpack_from("<H", payload, 33)[0]  # Offset 25+8
    voltage_batt_one_mv = struct.unpack_from("<H", payload, 35)[0]  # Offset 27+8
    voltage_batt_sum_mv = struct.unpack_from("<H", payload, 37)[0]  # Offset 29+8

    temp_batt1 = struct.unpack_from("b", payload, 41)[0]  # Offset 33+8
    temp_batt2 = struct.unpack_from("b", payload, 42)[0]  # Offset 34+8

    # OBC Block
    obc_activity = payload[50]  # Offset 42+8
    temp_x_plus = struct.unpack_from("b", payload, 51)[0]  # Offset 43+8
    temp_x_minus = struct.unpack_from("b", payload, 52)[0]  # Offset 44+8
    temp_y_plus = struct.unpack_from("b", payload, 53)[0]  # Offset 45+8
    temp_y_minus = struct.unpack_from("b", payload, 54)[0]  # Offset 46+8
    gnss_count = payload[55]  # Offset 47+8
    media_files_count = payload[58]  # Offset 50+8

    # COMMU Block
    vbus_voltage_mv = struct.unpack_from("<H", payload, 65)[0]  # Offset 57+8
    rssi_last = struct.unpack_from("b", payload, 69)[0]  # Offset 61+8
    rssi_min = struct.unpack_from("b", payload, 70)[0]  # Offset 62+8
    packets_sent = payload[73]  # Offset 65+8
    qso_received = payload[77]  # Offset 69+8

    print(
        f"Time: {time_unix} | Batt: {voltage_batt_sum_mv}mV | Media Files: {media_files_count}"
    )
    print(f"Temp Batt: {temp_batt1}/{temp_batt2}C | RSSI: {rssi_last}dBm")

    return True


FILE = "../files/INPUT.bin"

SYNC = 0x930B51DE
PREAMBLE = 0xAAAAAAAA
FRAME_LEN = 72
CRC_LEN = 2

# Read input file
with open(FILE, "rb") as f:
    data = f.read()

# Convert bytes to bits
bits = np.unpackbits(np.frombuffer(data, dtype=np.uint8))

# Create sync word bits
sync_bits = np.array([(SYNC >> (31 - i)) & 1 for i in range(32)], dtype=np.uint8)

count = 0
crc_ok = 0

# Store images by ID
images = {}

# Search sync words
for i in range(len(bits) - 32):
    if np.array_equal(bits[i : i + 32], sync_bits):
        # Get packet after sync
        start = i + 32
        frame_bits = bits[start : start + (FRAME_LEN + CRC_LEN) * 8]

        # Convert bits back to bytes
        frame = np.packbits(frame_bits).tobytes()
        # Remove scrambling
        frame = pn9_descramble(frame)

        # Build full packet
        full_packet = struct.pack(">I", PREAMBLE) + struct.pack(">I", SYNC) + frame

        # Check CRC
        crc = full_packet[80:82]

        calc_crc = crc16(full_packet[8:80])
        recv_crc = int.from_bytes(crc, "big")

        sync_img = struct.unpack(">I", full_packet[13:17])[0]

        if sync_img == 0x316F6B6F:
            continue

        count += 1

        if calc_crc == recv_crc:
            crc_ok += 1

            parse_geoscan_telemetry_packet(full_packet)

print(f"Всего найдено синхросло: {count}")
print(f"Совпавших CRC: {crc_ok}")
