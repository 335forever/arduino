#ifndef CUSTOM_COLORS_H
#define CUSTOM_COLORS_H

// Định nghĩa màu tùy chỉnh theo định dạng RGB565
#define CUSTOM_BLUE  0x5A9F   // Màu xanh dương tùy chỉnh (RGB 90, 180, 255)
#define CUSTOM_RED   0xF800   // Màu đỏ tùy chỉnh (RGB 255, 0, 0)
#define CUSTOM_GREEN 0x07E0   // Màu xanh lá tùy chỉnh (RGB 0, 255, 0)
#define CUSTOM_ORANGE 0xFDA0  // Màu cam tùy chỉnh (RGB 255, 165, 0)
#define CUSTOM_GRAY  0x8410   // Màu xám tùy chỉnh (RGB 128, 128, 128)

// Hàm chuyển đổi màu từ RGB888 sang RGB565 (nếu cần)
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

#endif
