import struct

SECTOR_SIZE = 512
total_sectors = 20480
sectors_per_cluster = 8
reserved_sectors = 1
num_fats = 2
root_entries = 512
sectors_per_fat = 16

img = bytearray(total_sectors * SECTOR_SIZE)

bpb = bytearray(512)
bpb[0:3] = b'\xEB\x3C\x90'
bpb[3:11] = b'T.B.M.K. '
struct.pack_into('<H', bpb, 11, SECTOR_SIZE)
struct.pack_into('<B', bpb, 13, sectors_per_cluster)
struct.pack_into('<H', bpb, 14, reserved_sectors)
struct.pack_into('<B', bpb, 16, num_fats)
struct.pack_into('<H', bpb, 17, root_entries)
struct.pack_into('<H', bpb, 19, total_sectors & 0xFFFF)
struct.pack_into('<B', bpb, 21, 0xF8)
struct.pack_into('<H', bpb, 22, sectors_per_fat)
struct.pack_into('<H', bpb, 24, 63)
struct.pack_into('<H', bpb, 26, 16)
struct.pack_into('<I', bpb, 28, 0)
struct.pack_into('<I', bpb, 32, 0)
struct.pack_into('<H', bpb, 510, 0xAA55)
img[0:512] = bpb

fat1 = reserved_sectors * SECTOR_SIZE
fat2 = fat1 + sectors_per_fat * SECTOR_SIZE
rds = (root_entries * 32 + 511) // SECTOR_SIZE
root = (reserved_sectors + num_fats * sectors_per_fat) * SECTOR_SIZE
data = root + rds * SECTOR_SIZE

img[fat1] = 0xF8
img[fat1+1] = 0xFF
img[fat1+2] = 0xFF
img[fat1+3] = 0xFF
img[fat2:fat2+4] = img[fat1:fat1+4]

ve = bytearray(32)
ve[0:11] = b'TBMK_DISK  '
ve[11] = 0x08
img[root:root+32] = ve

cdo = 0

def cf(name, ext, content, do):
    global cdo
    e = bytearray(32)
    nb = name.encode().upper()
    eb = ext.encode().upper()
    for i in range(8): e[i] = nb[i] if i < len(nb) else 0x20
    for i in range(3): e[8+i] = eb[i] if i < len(eb) else 0x20
    e[11] = 0x20
    cb = content.encode() if isinstance(content, str) else content
    struct.pack_into('<I', e, 28, len(cb))
    cluster = 2 + (cdo // SECTOR_SIZE) // sectors_per_cluster
    struct.pack_into('<H', e, 26, cluster)
    img[root+do:root+do+32] = e
    for i in range(len(cb)): img[data+cdo+i] = cb[i]
    struct.pack_into('<H', img, fat1+cluster*2, 0xFFFF)
    struct.pack_into('<H', img, fat2+cluster*2, 0xFFFF)
    used = ((len(cb)+511)//512 + sectors_per_cluster - 1) // sectors_per_cluster
    cdo += used * sectors_per_cluster * SECTOR_SIZE

cf('README','TXT','Welcome to T.B.M.K. FAT16!\r\nTest file.\r\n',32)
cf('HELLO','TXT','Hello from TBMK!\r\n',64)
cf('CONFIG','SYS','TBMK=1\r\n',96)

with open('disk.img','wb') as f:
    f.write(img)

print('[OK] disk.img created')