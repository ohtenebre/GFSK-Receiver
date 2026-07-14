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
def parse_geoscan_image_packet(payload: bytes):
    if len(payload) < 78:
        print(f"Ошибка: Размер пакета {len(payload)} байт, а должно быть 78.")
        return

    print("Геоскан: Пакет Изображения")

    # Read preamble
    preamble = struct.unpack(">I", payload[0:4])[0]
    print(f"Преамбула: {hex(preamble)} (Ожидается 0xAAAAAAAA)")

    # Read sync word
    sync1 = struct.unpack(">I", payload[4:8])[0]
    print(f"Синхрослово 1: {hex(sync1)} (Ожидается 0x930B51DE)")

    # Read header
    header = payload[8:13]
    print(f"Заголовок (HEX): {header.hex()}")

    # Image sync word
    sync_img = struct.unpack(">I", payload[13:17])[0]
    print(f"Синхрослово картинки: {hex(sync_img)} (Ожидается 0x316F6B6F)")

    # Image offset Little Endiad
    offset = struct.unpack("<I", payload[17:21])[0]
    # print(f"Смещение в файле: {offset} байт")

    # File number Little Endiad
    file_id = struct.unpack("<H", payload[21:23])[0]
    # print(f"ID файла (Номер картинки): {file_id}")

    # Image payload
    image_data = payload[23:76]
    print(f"Размер считанных данных: {len(image_data)} байт")
    print(f"Данные куска (HEX): {image_data.hex()[:30]}...")

    # Packet CRC
    packet_crc = struct.unpack(">H", payload[76:78])[0]
    print(f"CRC пакета: {hex(packet_crc)}")

    return file_id, offset, image_data


FILE = "INPUT_FILE"

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
        count += 1

        # Get packet after sync
        start = i + 32
        frame_bits = bits[start : start + (FRAME_LEN + CRC_LEN) * 8]

        # Convert bits back to bytes
        frame = np.packbits(frame_bits).tobytes()
        # Remove scrambling
        frame = pn9_descramble(frame)

        # Check CRC
        crc = frame[72:74]

        calc_crc = crc16(frame[:72])
        recv_crc = int.from_bytes(crc, "big")

        # Build full packet
        full_packet = struct.pack(">I", PREAMBLE) + struct.pack(">I", SYNC) + frame

        if calc_crc == recv_crc:
            crc_ok += 1
            # Get packet fields
            offset = struct.unpack("<I", full_packet[17:21])[0]
            file_id = struct.unpack("<H", full_packet[21:23])[0]

            # Get payload
            data = full_packet[23:76]

            print(f"offset: {offset}")
            print(f"id: {file_id}")

            # Create new image
            if file_id not in images:
                images[file_id] = bytearray()

            img = images[file_id]

            # Expand image buffer
            if len(img) < offset + len(data):
                img.extend(b"\x00" * (offset + len(data) - len(img)))

            # Put data to correct place
            img[offset : offset + len(data)] = data

            # parse_geoscan_image_packet(full_packet)

        # if crc_ok == 1:
        #     break

print(f"Всего найдено синхросло: {count}")
print(f"Совпавших CRC: {crc_ok}")

# Save images
for file_id, img in images.items():
    with open(f"image_{file_id}.bin", "wb") as f:
        f.write(img)
