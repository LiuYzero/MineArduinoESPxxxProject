from PIL import Image
import os
import struct
import socket
import time
"""
pic2socket
"""


def rgb_to_rgb565(r, g, b):
    """
    将RGB值(0-255)转换为RGB565格式(16位)
    """
    # 将8位RGB值转换为5-6-5位
    r_5bit = (r >> 3) & 0x1F  # 取高5位
    g_6bit = (g >> 2) & 0x3F  # 取高6位
    b_5bit = (b >> 3) & 0x1F  # 取高5位

    # 组合成16位的RGB565值
    rgb565 = (r_5bit << 11) | (g_6bit << 5) | b_5bit
    return rgb565


def process_png_to_rgb565(image_path):
    """
    处理PNG图片并输出每个像素的RGB和RGB565值
    """
    rgb565_pixels = []
    try:
        # 打开图片
        img = Image.open(image_path)

        # 转换为RGB模式（确保没有alpha通道）
        if img.mode != 'RGB':
            img = img.convert('RGB')

        # 获取图片尺寸
        width, height = img.size
        print(f"图片尺寸: {width} x {height}")
        print(f"总像素数: {width * height}")
        print("-" * 60)

        # 获取像素数据
        pixels = img.load()

        print("像素数据 (格式: [x,y] RGB(R,G,B) -> RGB565(十六进制, 十进制)):")
        print("=" * 60)

        # 遍历每个像素
        for y in range(height):
            for x in range(width):
                # 获取RGB值
                r, g, b = pixels[x, y]

                # 转换为RGB565
                rgb565_value = rgb_to_rgb565(r, g, b)

                # 输出结果
                rgb565_pixels.append((x,y,rgb565_value))


        print("=" * 60)

        # 返回图片对象以供后续使用
        return rgb565_pixels

    except Exception as e:
        print(f"处理图片时出错: {e}")
        return []


def create_rgb565_array(image_path, output_file=None):
    """
    创建RGB565数据的C语言数组格式
    """
    try:
        img = Image.open(image_path)
        if img.mode != 'RGB':
            img = img.convert('RGB')

        width, height = img.size
        pixels = img.load()

        if output_file is None:
            output_file = os.path.splitext(image_path)[0] + "_array.txt"

        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(f"// PNG图片: {image_path}\n")
            f.write(f"// 尺寸: {width} x {height}\n")
            f.write(f"const uint16_t image_data[{height}][{width}] = {{\n")

            for y in range(height):
                f.write("    {")
                row_data = []
                for x in range(width):
                    r, g, b = pixels[x, y]
                    rgb565_value = rgb_to_rgb565(r, g, b)
                    row_data.append(f"0x{rgb565_value:04X}")

                f.write(", ".join(row_data))
                if y < height - 1:
                    f.write("},\n")
                else:
                    f.write("}\n")

            f.write("};\n")

        print(f"RGB565数组已保存到: {output_file}")

    except Exception as e:
        print(f"创建数组时出错: {e}")


def split_rgb565_to_bytes(rgb565_value, endian='big'):
    """
    使用struct将RGB565值拆分为两个字节

    参数:
    - rgb565_value: RGB565值 (16位无符号整数)
    - endian: 字节序 ('big' 或 'little')

    返回:
    - (high_byte, low_byte): 两个字节的元组
    """
    # 使用struct将16位整数打包为2个字节
    if endian == 'big':
        # 大端序: 高字节在前
        packed = struct.pack('>H', rgb565_value)
    else:
        # 小端序: 低字节在前
        packed = struct.pack('<H', rgb565_value)

    # 解包为两个字节
    high_byte, low_byte = struct. unpack('BB', packed)
    return high_byte, low_byte

# 示例使用
if __name__ == "__main__":
    image_path = "pics_240x240/lingmeng.png"  # 修改为你的图片路径
    rgb565_pixels = process_png_to_rgb565(image_path)

    target_ip = "192.168.1.108"
    target_port = 5678

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((target_ip, target_port))
    for item in rgb565_pixels:
        print(f"x={item[0]}, y={item[1]}, color={item[2]}")
        high_byte, low_byte = split_rgb565_to_bytes(item[2],"big")
        data = struct.pack('4B', item[0], item[1], high_byte, low_byte)
        client_socket.send(data)
        # time.sleep(0.01)
    client_socket.close()