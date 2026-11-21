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

img[fat1] = 0xF8
img[fat1+1] = 0xFF
img[fat1+2] = 0xFF
img[fat1+3] = 0xFF
img[fat2:fat2+4] = img[fat1:fat1+4]

with open('disk.img','wb') as f:
    f.write(img)

print('[OK] Empty disk.img created')